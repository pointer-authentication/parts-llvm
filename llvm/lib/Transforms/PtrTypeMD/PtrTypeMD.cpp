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

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "../../Target/AArch64/PointerAuthentication.h"
using namespace llvm;

#define DEBUG_TYPE "PtrTypeMDPass"
namespace {
 
  struct PtrTypeMDPass : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    PtrTypeMDPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
        //errs() << "Hello: ";
        //errs().write_escaped(F.getName()) << '\n';
      for (auto &BB:F){
	//errs() << "Basic Block: ";
	//errs().write_escaped(F.getName()) << '\n';
          for (auto &I: BB){
                   //errs() << "Instruction: ";
                   //I.dump();
               if (I.getOpcode()==Instruction::Load || I.getOpcode()==Instruction::Store ){
                  errs() << "\n";
                  errs() << "Instruction: ";
                  I.dump();
                  errs().write_escaped(I.getOpcodeName(I.getOpcode()));
                  errs() << "\n";
                  Type * Ty = I.getOperand(0)->getType();
                  if(Ty->isPointerTy()){
                         if (PointerType * PT = dyn_cast<PointerType>(Ty)) {
                               Type* ty=PT->getElementType();
                               //errs()<<PT->getElementType()<<"\n";
                               bool fty= PT->getElementType()->isFunctionTy();
                               errs()<<fty<<"\n";
                               errs()<<"Pointer Type:"<<"\n";
                               ty->dump();
                               std::string type_str;
                               llvm::raw_string_ostream rso(type_str);
                               ty->print(rso);
                               auto &C = F.getContext();
                               Metadata* vals[2]={MDString::get(C, rso.str()), MDString::get(C, std::string (std::to_string(fty)))};
                               MDNode *N = MDNode::get(C,vals);
                               I.setMetadata("PAData", N);
                               errs() << "Metadata:"<<"\n";
                               errs() << cast<MDString>(I.getMetadata("PAData")->getOperand(0))->getString()<<"\n";
                               errs() << cast<MDString>(I.getMetadata("PAData")->getOperand(1))->getString()<<"\n";
                               errs()<<"\n";  
                         }
                      }
                   }
                 }
 	    }
      return true;
    }
  };
}
char PtrTypeMDPass::ID = 0;
static RegisterPass<PtrTypeMDPass> X("ptr-type-md-pass", "Pointer Type Metadata Pass");

