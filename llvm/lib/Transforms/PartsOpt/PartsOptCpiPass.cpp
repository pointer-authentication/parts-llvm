//===----------------------------------------------------------------------===//
//
// Author: Zaheer Gauhar <zaheer.gauhar@aalto.fi>
//         Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
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

#define DEBUG_TYPE "PartsOptCpiPass"

//#undef DEBUG_PA
//#define DEBUG_PA(x) x

namespace {

struct PartsOptCpiPass : public FunctionPass {
  static char ID;

  PartsLog_ptr log;

  PartsOptCpiPass() : FunctionPass(ID), log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool runOnFunction(Function &F) override;

  void fixDirectFunctionArgs(Function &F, Instruction &I);

  /**
   * Check the input Value V and recursively change inner parameters, or
   * if V itself needs to be replaced in the containing function, then
   * uses given builder to generate PACed value and returns said value.
   * @param M
   * @param I
   * @param V
   * @return
   */
  Value *generatePACedValue(Module *M, Instruction &I, Value *V) {
    const auto VType = V->getType();

    // We can directly skip if we don't need to do anything
    if (! VType->isPointerTy())
      return nullptr;

    // We want to PAC function, but ignore intrinsics.
    if (isa<Function>(V) && !dyn_cast<Function>(V)->isIntrinsic()) {
      // Generate Builder for inserting pa_pacia
      IRBuilder<> Builder(&I);
      // Get pa_pacia declaration for correct input type
      auto pacia = Intrinsic::getDeclaration(M, Intrinsic::pa_pacia, { VType });
      // Get type_id as Constant
      auto typeIdConstant = PartsTypeMetadata::idConstantFromType(M->getContext(), VType);
      // Insert intrinsics to generate PACed Value and return it
      return Builder.CreateCall(pacia, { V, typeIdConstant }, "");
    }

    // For bitcast we may want to PAC the input pointer
    if (isa<BitCastOperator>(V)) {
      auto BC = dyn_cast<BitCastOperator>(V);
      auto paced = generatePACedValue(M, I, BC->getOperand(0));

      if (paced != nullptr) {
        BC->setOperand(0, paced);
      }

      // We can return nullptr, since all changes have already been made
      return nullptr;
    }

    return nullptr;
  }

  inline void replaceDirectFuncOperand(Function &F, Instruction &I, Value *O, CallInst *CI, unsigned i) {
    auto paced_arg = PartsIntr::pac_pointer(F, I, O);
    CI->setOperand(i, paced_arg);
  }
};

} // anonymous namespace

char PartsOptCpiPass::ID = 0;
static RegisterPass<PartsOptCpiPass> X("parts-opt-cpi", "PARTS CPI pass");

bool PartsOptCpiPass::runOnFunction(Function &F) {
  if (!PARTS::useFeCfi())
    return false;

  for (auto &BB:F){
    for (auto &I: BB) {
      const auto IOpcode = I.getOpcode();

      switch(IOpcode) {
        default:
          break;
        case Instruction::Store: {
          auto SI = dyn_cast<StoreInst>(&I);
          assert(SI != nullptr && "this should always be a store instruction, maybe remove this assert?");

          auto paced = generatePACedValue(F.getParent(), I, SI->getValueOperand());
          if (paced != nullptr) {
            log->inc(DEBUG_TYPE ".PacStoreFunction", true, F.getName()) << "PACing store of function address\n";
            SI->setOperand(0, paced);
          }
          break;
        }
        case Instruction::Load: {
          break;
        }
        case Instruction::Select: {
          for (unsigned i = 0, end = I.getNumOperands(); i < end; ++i) {
            auto paced = generatePACedValue(F.getParent(), I, I.getOperand(i));
            if (paced != nullptr) {
              log->inc(DEBUG_TYPE ".PacSelect", true, F.getName()) << "PACing store of function address\n";
              I.setOperand(i, paced);
            }
          }
          break;
        }
        case Instruction::Call: {
          fixDirectFunctionArgs(F, I);

          auto CI = dyn_cast<CallInst>(&I);

          if (CI->getCalledFunction() == nullptr) {
            auto O = CI->getCalledValue();

            if (isa<InlineAsm>(O)) {
              // Ignore inline asm
              DEBUG_PA(log->info() << "ignoring inline asm indirect call\n");
            } else if (isa<Function>(O) && dyn_cast<Function>(O)->isIntrinsic()) {
              // Ignore intrinsic calls
              DEBUG_PA(log->info() << "ignoring intrinsic call\n");
            } else if (isa<BitCastOperator>(O)) {
              // Ignore bitcast operator
              DEBUG_PA(log->info() << "ignoring bitcast operator\n");
            } else {
              log->inc(DEBUG_TYPE ".IndirectCall", true, F.getName()) << "found indirect call!!!!\n";
              auto aut_arg = PartsIntr::aut_pointer(F, I, O);
              CI->setCalledFunction(aut_arg);
            }
          }
          break;
        }
      }
    }
  }

  return true;
}

void PartsOptCpiPass::fixDirectFunctionArgs(Function &F, Instruction &I) {
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
