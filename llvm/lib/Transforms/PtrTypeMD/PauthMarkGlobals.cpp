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
#include "../../Target/AArch64/PointerAuthentication.h"

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
  int marked_data_pointers = 0;
  int marked_code_pointers = 0;

  // Automatically annotate pointer globals
  for (auto GI = M.global_begin(); GI != M.global_end(); GI++) {
    auto Ty = GI->getOperand(0)->getType();

    if (Ty->isPointerTy() && !GI->hasSection()) {
      auto type_id = PA::createPauthTypeId(Ty);

      if (PA::isInstruction(type_id)) {
        marked_code_pointers++; // This should eventually be put in .code_pauth
        GI->setSection(".data_pauth");
      } else {
        marked_data_pointers++;
        GI->setSection(".data_pauth");
      }
      // TODO: add the type_ids into their own sections (.data_pauth_type_id, .code_pauth_type_id)
    }
  }

  DEBUG(errs() << getPassName() << ": moved " << marked_data_pointers << "+" << marked_code_pointers <<
               " globals to pauth data/code section(s)\n");

  return (marked_code_pointers+marked_code_pointers) > 0;
}
