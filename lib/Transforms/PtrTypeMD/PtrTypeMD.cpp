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
#include "llvm/IR/Constant.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "MDclass.h"
#include "../../Target/AArch64/PointerAuthentication.h"

using namespace llvm;
using namespace llvm::PA;

#define DEBUG_TYPE "PtrTypeMDPass"
#define TAG KBLU DEBUG_TYPE ": "

namespace {

struct PtrTypeMDPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  MDclass md=MDclass();
  PtrTypeMDPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  //void addToStore(Function &F, Instruction &I);
  //void addToLoad(Function &F, Instruction &I);

  /** Function to assert the types of instruction operands to be equal */
  //void assertPtrType(Instruction &I);

  /** Function to set metadata on an instruction */
  //void md_fn(Function &F, Instruction &I, Type* ptrTy, bool fty);

  //Function to check if the pointer of an instruction operand points to a function instead of data
  //Type* isFnType(Type* Ty);

  //Function to insert NOP instructions
  //void insertNops(Function &F, Instruction &I);
};

} // anonyous namespace

char PtrTypeMDPass::ID = 0;
static RegisterPass<PtrTypeMDPass> X("ptr-type-md-pass", "Pointer Type Metadata Pass");


bool PtrTypeMDPass::runOnFunction(Function &F) {
  DEBUG_PA_OPT(&F, do { errs() << TAG << "Function: "; errs().write_escaped(F.getName()) << "\n"; } while(false));

  auto &C = F.getContext();

  for (auto &BB:F){
    DEBUG_PA_OPT(&F, do { errs() << TAG << "\tBasicBlock: "; errs().write_escaped(BB.getName()) << '\n'; } while(false));

    for (auto &I: BB) {
      DEBUG_PA_OPT(&F, do { errs() << TAG << "\t\t"; I.dump(); } while(false));

      const auto IOpcode = I.getOpcode();

      MDNode *MD = nullptr;

      switch(IOpcode) {
        default:
          break;
        case Instruction::Store:
          MD = createPauthMDNode(C, I.getOperand(0)->getType());
          break;
        case Instruction::Load:
          MD = createPauthMDNode(C, I.getType());
          break;
        case Instruction::Call:

          if (isa<CallInst>(I)) {
            const CallInst *CI = dyn_cast<CallInst>(&I);
            if (CI->getCalledFunction() == nullptr) {
              DEBUG_PA_OPT(&F, errs() << TAG << KGRN << "\t\t\t found indirect call!!!!\n");
              MD = createPauthMDNode(C, I.getOperand(0)->getType());
            } else {
              MD = createPauthMDNode(C, type_id_Ignore);
            }
          }
          break;
      }

      if (MD != nullptr) {
        I.setMetadata(Pauth_MDKind, MD);
        DEBUG_PA_OPT(&F, do { errs() << TAG << "\t\t\t adding metadata "; I.getMetadata(Pauth_MDKind)->dump(); } while(0));
      } else {
        DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\t skipping\n");
      }
    }
  }

  return true;
}

/*
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

  DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\tSetting metadata: ");
  DEBUG_PA_OPT(&F, errs() << TAG << cast<MDString>(I.getMetadata("PAData")->getOperand(0))->getString() << " ");
  DEBUG_PA_OPT(&F, errs() << TAG << cast<MDString>(I.getMetadata("PAData")->getOperand(1))->getString() << "\n");
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

*/
