//===----------------------------------------------------------------------===//
//
// Authors: Zaheer Ahmed Gauhar
//          Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/PARTS/PartsIntr.h>
#include "llvm/IR/IRBuilder.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsLog.h"

using namespace llvm;
using namespace llvm::PARTS;

#define DEBUG_TYPE "PartsCfi"

namespace {

struct PartsCfi : public FunctionPass {
  static char ID;

  PartsLog_ptr log;

  PartsCfi() : FunctionPass(ID), log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool runOnFunction(Function &F) override;

  void fixDirectFunctionArgs(Function &F, Instruction &I);
};

} // anonymous namespace

char PartsCfi::ID = 0;
static RegisterPass<PartsCfi> X("parts-fecfi-pass", "PARTS CFI pass");

bool PartsCfi::runOnFunction(Function &F) {
  if (!PARTS::useFeCfi())
    return false;

  for (auto &BB:F){
    for (auto &I: BB) {
      DEBUG_PA(log->debug() << F.getName() << "->" << BB.getName() << "->" << I << "\n");

      const auto IOpcode = I.getOpcode();

      PartsTypeMetadata_ptr MD = nullptr;

      switch(IOpcode) {
        default:
          break;
        case Instruction::Store:
          MD = PartsTypeMetadata::get(I.getOperand(0)->getType());
          if (MD->isCodePointer()) {

            auto O = I.getOperand(1);
            if (PartsTypeMetadata::TyIsCodePointer(O->getType()) && isa<Function>(O)) {
              log->inc(DEBUG_TYPE ".StoreFunction", true, F.getName()) << "PACing store of function address\n";
              //auto paced_arg = PartsIntr::pac_code_pointer(F, I, O);
              //I.setOperand(1, paced_arg);
            } else {
              // Assume th pointer is already PACed, in which case we need to rePAC
              log->inc(DEBUG_TYPE ".StoreCodePointer", true, F.getName()) << "PACing store of code-pointer\n";
              //auto aut_arg = PartsIntr::load_aut_pointer(F, I, MD);
              //auto paced_arg = PartsIntr::pac_code_pointer(F, I, aut_arg);
              //I.setOperand(1, paced_arg);
            }
          }
          break;
        case Instruction::Load:
          break;
        case Instruction::Call:
          fixDirectFunctionArgs(F, I);

          auto CI = dyn_cast<CallInst>(&I);

          if (CI->getCalledFunction() == nullptr) {

            //auto O = I.getOperand(0);
            auto O = CI->getCalledValue();

            if (isa<InlineAsm>(O)) {
              DEBUG_PA(log->info() << "ignoring inline asm indirect call\n");
              // Ignore inline asm
              break;
            }

            log->inc(DEBUG_TYPE ".IndirectCall", true, F.getName()) << "      found indirect call!!!!\n";

            auto paced_arg = PartsIntr::pac_code_pointer(F, I, O);
            CI->setOperand(0, paced_arg);
          }

          break;
      }
    }
  }

  return true;
}

void PartsCfi::fixDirectFunctionArgs(Function &F, Instruction &I) {
  if (!PARTS::useFeCfi())
    return;

  auto CI = dyn_cast<CallInst>(&I);

  for (auto i = 0U; i < CI->getNumOperands() - 1; i++) {
    auto O = CI->getOperand(i);

    if (PartsTypeMetadata::TyIsCodePointer(O->getType()) && isa<Function>(O)) {
      auto paced_arg = PartsIntr::pac_code_pointer(F, I, O);
      CI->setOperand(i, paced_arg);
    }
  }
}

