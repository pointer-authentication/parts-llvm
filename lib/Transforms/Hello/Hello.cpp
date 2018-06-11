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
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "../../Target/AArch64/AArch64Pa.h"
using namespace llvm;

#define DEBUG_TYPE "hello"

STATISTIC(HelloCounter, "Counts number of functions greeted");

namespace {
  // Hello - The first implementation, without getAnalysisUsage.
  struct Hello : public FunctionPass {
    static char ID; // Pass identification, replacement for typeid
    Hello() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {
      ++HelloCounter;
      errs() << "Hello: ";
      errs().write_escaped(F.getName()) << '\n';

      // Iterate through the Basic Blocks
      for (auto &BB : F) {
          errs() << "  Basic block: ";
          errs().write_escaped(BB.getName()) << '\n';

          // Iterate throughy the Instructions
          for (auto &I : BB) {
              errs() << "    Instruction: ";
              I.dump();
              auto &C = F.getContext();
              MDNode *N = MDNode::get(C, MDString::get(C, "howdy"));
              I.setMetadata(PAMetaDataKind, N);
              errs() << "**************adding metadata to store\n";
          }
      }

      return false;
    }
  };
}

char Hello::ID = 0;
static RegisterPass<Hello> X("hello", "Hello World Pass");

/* INITIALIZE_PASS_BEGIN(Hello, "hello", "Gathering Function info",  false, false) */
/* /1* INITIALIZE_PASS_DEPENDENCY(DominatorTree) *1/ */
/* INITIALIZE_PASS_END(Hello, "hello", "gathering function info", false, false) */

/* FunctionPass *llvm::createFunctionInfoPass() { return new Hello(); } */
