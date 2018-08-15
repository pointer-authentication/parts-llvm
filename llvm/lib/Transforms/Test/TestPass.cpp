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

#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "MDclass.h"
#include "../../Target/AArch64/PointerAuthentication.h"

#include <iomanip>
#include <sstream>

extern "C"{
#include "include/sha3.h"
}

using namespace llvm;
using namespace llvm::PA;

#define DEBUG_TYPE "TestPass"
#define TAG KBLU DEBUG_TYPE ": "

namespace {

/*
#ifdef __cplusplus
extern "C"{
#endif
mbedtls_sha3_context c;
mbedtls_sha3_type_t type= MBEDTLS_SHA3_256;
void mbedtls_sha3_init(mbedtls_sha3_context *ctx);
int mbedtls_sha3_starts(mbedtls_sha3_context *ctx, mbedtls_sha3_type_t type);
#ifdef __cplusplus
}
#endif
*/

struct TestPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  MDclass md=MDclass();
  TestPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  void addToStore(Function &F, Instruction &I);
  void addToLoad(Function &F, Instruction &I);

  /** Function to assert the types of instruction operands to be equal */
  void assertPtrType(Instruction &I);

  /** Function to set metadata on an instruction */
  void md_fn(Function &F, Instruction &I, Type* ptrTy, bool fty);

  //Function to check if the pointer of an instruction operand points to a function instead of data
  Type* isFnType(Type* Ty);

  //Function to insert NOP instructions
  void insertNops(Function &F, Instruction &I);

  //Function to calculate sha3_256 hash of the operand type
  void sha3calc(Type* Ty);
};

} // anonyous namespace

char TestPass::ID = 0;
static RegisterPass<TestPass> X("test-pass", "Test Pass");


bool TestPass::runOnFunction(Function &F) {
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
      }

      if (MD != nullptr) {
        I.setMetadata(Pauth_MDKind, MD);
        DEBUG_PA_OPT(&F, do { errs() << TAG << "\t\t\t adding metadata "; I.getMetadata(Pauth_MDKind)->dump(); } while(0));

//Re check instruction opcode for getting the appropriate operand type, can be done more efficiently by using previously found type
		if (!md.getFPtrType()){
			Type* raw_type = nullptr;
			if (IOpcode==Instruction::Load)
				raw_type=I.getOperand(0)->getType();
			else if (IOpcode==Instruction::Store)
				raw_type=I.getOperand(1)->getType();
		    //Call to sha3 calculator function defined below
			sha3calc(raw_type);
		}
//***********************
      } else {
        DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\t skipping\n");
      }
    }
  }

  return true;
}
//Function to calculate sha3 hash and display it as string 

void TestPass::sha3calc(Type* ty){

	std::string type_str;
	llvm::raw_string_ostream rso(type_str);
	ty->print(rso);
	errs()<<rso.str()<<"\n";

	mbedtls_sha3_context c;
	mbedtls_sha3_type_t type= MBEDTLS_SHA3_256;
	mbedtls_sha3_init(&c);
	const unsigned char *input=reinterpret_cast<const unsigned char*>(rso.str().c_str());
	unsigned char *output= new unsigned char[32]();
	int hash=mbedtls_sha3(input, sizeof(input), type, output);

	std::stringstream ss;
	ss << std::hex << std::setfill('0');
	for (int i = 0; i < 32; ++i)
	{
	    ss << std::setw(2) << static_cast<unsigned>(output[i]);
		}
	errs()<<hash<<"\n";
	errs()<<ss.str()<<"\n";
}
//**************************
void TestPass::assertPtrType(Instruction &I)
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
void TestPass::md_fn(Function &F, Instruction &I, Type* ptrTy, bool fty) {
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
Type* TestPass::isFnType(Type* Ty){
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
void TestPass::insertNops(Function &F, Instruction &I){
  auto &C= F.getContext();
  Value* zero = ConstantInt::get(Type::getInt32Ty(C),0);
  auto* newInst = BinaryOperator::Create(Instruction::Add, zero, zero, "nop", I.getNextNode());
  MDNode *N= MDNode::get(C, MDString::get(C, "Nop Instruction"));
  newInst->setMetadata("PAData", N);
}

