//===- PtrTypeMD.cpp - Code For Pointer Type Metadata ---------------------===/
//
//                     The LLVM Compiler Infrastructure
//
// This code is released under Apache 2.0 license.
// Author: Zaheer Ahmed Gauhar
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
//===----------------------------------------------------------------------===//
//
// This file implements Pointer type extraction for load and store 
// instructions and differentiates between pointers to data and functions.
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/IRBuilder.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "MDclass.h"
#include "../../Target/AArch64/PointerAuthentication.h"

using namespace llvm;

#define DEBUG_TYPE "PtrTypeMDPass"
namespace {

struct PtrTypeMDPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  Function *funcFixMain = nullptr;

  MDclass md=MDclass();
  PtrTypeMDPass() : FunctionPass(ID) {}

  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;

  bool runOnFunction(Function &F) override;

  void pacMainArgs(Function &F);

  /** Function to assert the types of instruction operands to be equal */
  void assertPtrType(Instruction &I);

  /** Function to set metadata on an instruction */
  void md_fn(Function &F, Instruction &I, Type* ptrTy, bool fty);

  //Function to check if the pointer of an instruction operand points to a function instead of data
  Type* isFnType(Type* Ty);

  //Function to insert NOP instructions
  void insertNops(Function &F, Instruction &I);
};

} // anonyous namespace

char PtrTypeMDPass::ID = 0;
static RegisterPass<PtrTypeMDPass> X("ptr-type-md-pass", "Pointer Type Metadata Pass");


bool PtrTypeMDPass::runOnFunction(Function &F) {
  DEBUG_PA_OPT(errs() << "Function: ");
  DEBUG_PA_OPT(errs().write_escaped(F.getName()) << "\n");

  for (auto &BB:F){
    DEBUG_PA_OPT(errs() << "\tBasicBlock: ");
    DEBUG_PA_OPT(errs().write_escaped(BB.getName()) << '\n' << KNRM);

    for (auto &I: BB) {
      DEBUG_PA_OPT(errs() << "\t\t");
      DEBUG_PA_OPT(I.dump());

      const auto IOpcode = I.getOpcode();

      if (IOpcode == Instruction::Load || IOpcode == Instruction::Store) {
        const auto *IType = (IOpcode == Instruction::Store
                            ? I.getOperand(0)->getType()
                            : I.getType()
        );

        DEBUG_PA_OPT(errs() << "\t\t\t*** Found a " << (IOpcode == Instruction::Store ? "store" : "load") << ":");
        DEBUG_PA_OPT(I.dump());

        if (IType->isPointerTy()) {
          DEBUG_PA_OPT(errs() << "\t\t\t*** it's a pointer!\n");

          md.setFPtrType(false);
          assertPtrType(I);

          bool fty=md.getFPtrType();
          md_fn(F,I,md.getPtrType(),fty);
        }
      }
    }
  }

  if (F.getName().equals("main"))
    pacMainArgs(F);

  return true;
}

bool PtrTypeMDPass::doInitialization(Module &M)
{

  Type* types[2];
  types[0] = Type::getInt32Ty(M.getContext());
  types[1] = PointerType::get(Type::getInt8PtrTy(M.getContext()), 0);
  ArrayRef<Type*> params(types, 2);
  auto result = Type::getVoidTy(M.getContext());

  FunctionType* signature = FunctionType::get(result, params, false);
  funcFixMain = Function::Create(signature, Function::ExternalLinkage, "__pauth_pac_main_args", &M);

  errs() << "Created new function with " << signature->getNumParams() << " params\n";

  return true;
}

bool PtrTypeMDPass::doFinalization(Module &M) {
  return true;
}

void PtrTypeMDPass::pacMainArgs(Function &F) {
  assert(F.getName().equals("main"));

  errs() << "Function arguments start\n";
  for (auto arg = F.arg_begin(); arg != F.arg_end(); arg++) {
    arg->dump();
  }
  errs() << "Function arguments end\n";

  auto AI = F.arg_begin();
  if (AI == F.arg_end() || AI->getType()->getTypeID() != Type::IntegerTyID) {
    errs() << "first argument not an integer\n";
    return;
  }
  auto &argc = *AI++;
  if (AI == F.arg_end() || AI->getType()->getTypeID() != Type::PointerTyID) {
    errs() << "second argument not a char **\n";
    return;
  }
  auto &argv = *AI++;
  if (AI != F.arg_end()) {
    errs() << "unexpected arguments";
    return;
  }

  auto &B = F.getEntryBlock();
  auto &I = *B.begin();

  /*
  (void)argc;
  (void)argv;
  (void)I;
   */

  std::vector<Value*> args(0);
  args.push_back(&argc);
  args.push_back(&argv);

  errs() << "Inserting call with " << args.size() << " params\n";

  IRBuilder<> Builder(&I);
  Builder.CreateCall(funcFixMain, args);
}

void PtrTypeMDPass::assertPtrType(Instruction &I)
{
  int ops=I.getNumOperands();
  Type* Ty=I.getOperand(0)->getType();
  Type* found_ty=isFnType(Ty);

  for (int i=1; i<ops; i++){
    Type* next_ty=I.getOperand(i)->getType();
    Type* next_found_ty=isFnType(next_ty);
    if (found_ty!=next_found_ty){
      errs()<<"Exception: Type mismatch between operands\n";
      break;
    }
    else{
      found_ty=next_found_ty;
    }
  }
  md.setPtrType(found_ty);
}
void PtrTypeMDPass::md_fn(Function &F, Instruction &I, Type* ptrTy, bool fty) {
  auto &C = F.getContext();

  std::string type_str;
  llvm::raw_string_ostream rso(type_str);
  ptrTy->print(rso);

  Metadata* vals[2] = {
      MDString::get(C, rso.str()),
      MDString::get(C, std::string(std::to_string(fty)))
  };

  MDNode *N = MDNode::get(C,vals);
  I.setMetadata("PAData", N);

  DEBUG_PA_OPT(errs() << "\t\t\tSetting metadata: ");
  DEBUG_PA_OPT(errs() << cast<MDString>(I.getMetadata("PAData")->getOperand(0))->getString() << " ");
  DEBUG_PA_OPT(errs() << cast<MDString>(I.getMetadata("PAData")->getOperand(1))->getString() << "\n");
}

//Function to check if the pointer of an instruction operand points to a function instead of data
Type* PtrTypeMDPass::isFnType(Type* Ty){
  if (PointerType * PT = dyn_cast<PointerType>(Ty)) {
    Type* ty=PT;
    //bool fty=false; //is function pointer type or not
    if (PointerType* pt = dyn_cast<PointerType>(ty)) {
      int i=0;
      do {
        Type* pointedType = pt->getPointerElementType();
        //errs()<<"Iteration:";
        //errs()<<i<<"\n";
        ++i;
        if (pointedType->isFunctionTy()) {
          //errs()<<"Found function pointer type"<<"\n";
          md.setFPtrType(pointedType->isFunctionTy());
        }
        // This may be a pointer to a pointer to ...
        ty = pointedType;
      } while ((pt = dyn_cast<PointerType>(ty)));
    }
    return ty;
  }
  return Ty;
}

//Function to insert NOP instructions
void PtrTypeMDPass::insertNops(Function &F, Instruction &I){
  auto &C= F.getContext();
  Value* zero = ConstantInt::get(Type::getInt32Ty(C),0);
  auto* newInst = BinaryOperator::Create(Instruction::Add, zero, zero, "nop", I.getNextNode());
  MDNode *N= MDNode::get(C, MDString::get(C, "Nop Instruction"));
  newInst->setMetadata("PAData", N);
}

