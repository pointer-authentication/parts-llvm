// PauthMarkGlobals.cpp - Code for Pointer Authentication
//
//                     The LLVM Compiler Infrastructure
//
// This code is released under Apache 2.0 license.
// Author: Hans Liljestrand
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//

#include <llvm/IR/IRBuilder.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "../../Target/AArch64/AArch64PARTS/PointerAuthentication.h"

using namespace llvm;

#define DEBUG_TYPE "PauthOptPauthMarkGlobals"
#define TAG KYEL DEBUG_TYPE ": "

namespace {

struct PauthMarkGlobals: public ModulePass {
  static char ID; // Pass identification, replacement for typeid

  PauthMarkGlobals() : ModulePass(ID) {}

  bool runOnModule(Module &F) override;
};

} // anonyous namespace

char PauthMarkGlobals::ID = 0;
static RegisterPass<PauthMarkGlobals> X("pauth-markglobals", "PAC argv for main call");

bool PauthMarkGlobals::runOnModule(Module &M)
{
  int marked = 0;

  // Automatically annotate pointer globals
  for (auto GI = M.global_begin(); GI != M.global_end(); GI++) {
    if (GI->getOperand(0)->getType()->isPointerTy() && !GI->hasSection()) {
      GI->setSection(".data_pauth");
      marked++;
    }
  }

  DEBUG(errs() << getPassName() << ": moved " << marked << " globals to pauth section(s)\n");

  return marked > 0;
}
