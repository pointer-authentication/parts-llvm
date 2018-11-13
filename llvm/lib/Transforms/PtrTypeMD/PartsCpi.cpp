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

struct PartsCpi : public FunctionPass {
  static char ID;

  PartsLog_ptr log;

  PartsCpi() : FunctionPass(ID), log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool runOnFunction(Function &F) override;

  PartsTypeMetadata_ptr createCallMetadata(Function &F, Instruction &I);
  void fixDirectFunctionArgs(Function &F, Instruction &I);

  inline void replaceDirectFuncOperand(Function &F, Instruction &I, Value *O, CallInst *CI, unsigned i) {
    auto paced_arg = PartsIntr::pac_pointer(F, I, O);
    CI->setOperand(i, paced_arg);
  }
};

} // anonymous namespace

char PartsCpi::ID = 0;
static RegisterPass<PartsCpi> X("parts-fecfi-pass", "PARTS CFI pass");

bool PartsCpi::runOnFunction(Function &F) {
  if (!PARTS::useFeCfi())
    return false;

  auto &C = F.getContext();

  for (auto &BB:F){
    for (auto &I: BB) {
      DEBUG_PA(log->debug() << F.getName() << "->" << BB.getName() << "->" << I << "\n");

      const auto IOpcode = I.getOpcode();

      PartsTypeMetadata_ptr MD = nullptr;

      switch(IOpcode) {
        default:
          break;
        case Instruction::Store: {
          auto SI = dyn_cast<StoreInst>(&I);
          assert(SI != nullptr);

          auto PO = SI->getPointerOperand();
          auto VO = SI->getValueOperand();
          MD = PartsTypeMetadata::get(PO->getType());

          if (isa<Function>(VO)) {
            log->inc(DEBUG_TYPE ".StoreFunction", true, F.getName()) << "PACing store of function address\n";

            auto paced_arg = PartsIntr::pac_pointer(F, I, VO);
            SI->setOperand(0, paced_arg);

            break;
          }

          if (isa<Constant>(VO)) {
            log->inc(DEBUG_TYPE ".StoreConstant", true, F.getName()) << "ignoring store of constant\n";
            break;
          }

          // Assume th pointer is already PACed
          log->inc(DEBUG_TYPE ".StoreCodePointer", true, F.getName()) << "ignoring re-store of code-pointer\n";
          break;
        }
        case Instruction::Load:
          break;
        case Instruction::Call: {
          fixDirectFunctionArgs(F, I);

          MD = createCallMetadata(F, I);

          if (MD != nullptr) {
            MD->attach(C, I);
            log->inc(DEBUG_TYPE ".MetadataAdded", !MD->isIgnored()) << "adding metadata: " << MD->toString() << "\n";
          } else {
            log->inc(DEBUG_TYPE ".MetadataMissing") << "missing metadata\n";
          }

#ifdef REMOVE_THIS_MAYBE
          auto CI = dyn_cast<CallInst>(&I);

          if (CI->getCalledFunction() == nullptr) {
            auto O = CI->getCalledValue();

            if (isa<InlineAsm>(O)) {
              DEBUG_PA(log->info() << "ignoring inline asm indirect call\n");
              // Ignore inline asm
              break;
            }

            //log->inc(DEBUG_TYPE ".IndirectCall", true, F.getName()) << "      found indirect call!!!!\n";

            //auto paced_arg = PartsIntr::pac_code_pointer(F, I, O);
            //CI->setOperand(0, paced_arg);
          }
#endif
          break;
        }
      }
    }
  }

  return true;
}

void PartsCpi::fixDirectFunctionArgs(Function &F, Instruction &I) {
  if (!PARTS::useFeCfi())
    return;

  auto CI = dyn_cast<CallInst>(&I);

  for (auto i = 0U; i < CI->getNumArgOperands(); i++) {
    auto O = CI->getArgOperand(i);

    // First make sure that we're dealing with a function pointer
    if (PartsTypeMetadata::TyIsCodePointer(O->getType())) {
      if (isa<Function>(O)) {
        // The argument is a function address directly taken i.e., func(printf).
        if (!dyn_cast<Function>(O)->isIntrinsic()) {
          // Ignore intrinsics! FIXME: Or should we?
          replaceDirectFuncOperand(F, I, O, CI, i);
        }
      } else if (isa<BitCastOperator>(O)) {
        // This is a bitcast, so src might be directly taken function
        auto BC = dyn_cast<BitCastOperator>(O);
        auto BCO = BC->getOperand(0);

        if (isa<Function>(BCO)) {
          if (!dyn_cast<Function>(BCO)->isIntrinsic()) {
            // Ignore intrinsics! FIXME: Or should we?
            replaceDirectFuncOperand(F, I, O, CI, i);
          }
        }
      }
    }
  }
}

PartsTypeMetadata_ptr PartsCpi::createCallMetadata(Function &F, Instruction &I) {
  assert(isa<CallInst>(I));

  PartsTypeMetadata_ptr MD;

  auto CI = dyn_cast<CallInst>(&I);

  if (CI->getCalledFunction() == nullptr) {
    log->inc(DEBUG_TYPE ".IndirectCallMetadataFound", true, F.getName()) << "      found indirect call!!!!\n";
    MD = PartsTypeMetadata::get(I.getOperand(0)->getType());
  } else {
    MD = PartsTypeMetadata::getIgnored();
  }

  return MD;
}
