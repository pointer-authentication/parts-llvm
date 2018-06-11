//===- Hello.cpp - Example code from "Writing an LLVM Pass" ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements two versions of the LLVM "Hello World" pass described
// in docs/WritingAnLLVMPass.html
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "Test_pass"
namespace {
  
    struct Test_pass : public FunctionPass {
      static char ID; // Pass identification, replacement for typeid
      Test_pass() : FunctionPass(ID) {}
  
      bool runOnFunction(Function &F) override {
          //errs() << "Hello: ";
          //errs().write_escaped(F.getName()) << '\n';
        for (auto &BB:F){
          //errs() << "Basic Block: ";
          //errs().write_escaped(F.getName()) << '\n';
          for (auto &I: BB){
                  //errs() << "Instruction: ";
                  //I.dump();
	      if (I.getOpcode()==Instruction::Load){
                 errs() << "\n";
                 I.dump();
                 errs().write_escaped(I.getOpcodeName(I.getOpcode()));
                 errs() << "\n";
                 Type * Ty = I.getType();
  		 Ty->dump();
		 if(Ty->isPointerTy()){
                 	if (PointerType * PT = dyn_cast<PointerType>(Ty)) {
                        	 Type* ty=PT->getElementType();
	                         //errs()<<PT->getElementType()<<"\n";
        	                 errs()<<PT->getElementType()->isFunctionTy()<<"\n";
                	         ty->dump();
                 	}
		 }
                 auto &C = F.getContext();
                 MDNode *N = MDNode::get(C,MDString::get(C,"Metadata"));
		 //MDKind mdKind=C->RegisterMDKind("load");
                 I.setMetadata("a", N);
                 //errs() << cast<MDString>(I.getMetadata("a")->getOperand(0))->getString();
                 //errs()<<"\n";
              }
	     else if (I.getOpcode()==Instruction::Store){
                 errs() << "\n";
                 I.dump();
                 errs().write_escaped(I.getOpcodeName(I.getOpcode()));
                 errs() << "\n";
                 Type * Ty = I.getOperand(0)->getType();
  		 Ty->dump();
		 if(Ty->isPointerTy()){
	                if (PointerType * PT = dyn_cast<PointerType>(Ty)) {
                         	Type* ty=PT->getElementType();
	                        //errs()<<PT->getElementType()<<"\n";
        	                errs()<<PT->getElementType()->isFunctionTy()<<"\n";
                	        ty->dump();
                 	}
		 }
                 auto &C = F.getContext();
                 MDNode *N = MDNode::get(C,MDString::get(C,"Metadata"));
		 //MDKind mdKind=C->RegisterMDKind("load");
                 I.setMetadata("a", N);
                 //errs() << cast<MDString>(I.getMetadata("a")->getOperand(0))->getString();
                 //errs()<<"\n";
               } 
 
            }
         }
       return false;
     }
   };
 }


char Test_pass::ID = 0;
static RegisterPass<Test_pass> X("test_pass", "Test Pass");

