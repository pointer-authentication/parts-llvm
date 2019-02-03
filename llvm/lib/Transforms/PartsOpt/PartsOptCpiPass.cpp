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

#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsLog.h"
#include "llvm/PARTS/PartsTypeMetadata.h"

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

  /**
   * Check the input Value V and recursively change inner parameters, or
   * if V itself needs to be replaced in the containing function, then
   * uses given builder to generate PACed value and returns said value.
   * @param M
   * @param I
   * @param V
   * @return
   */
  Value *generatePACedValue(Module *M, Instruction &I, Value *V);
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
          /*
           * For functions we need to do two things:
           * 1: Make sure any function args are PACed (e.g., ~ 'func(printf)' -> 'func(pacia(printf)))';
           * 2: Make sure indirect function calls are authenticated.
           * FIXME: handle pointers InlineAsm
           * FIXME: handle indirect calls using BitCastOperator (could perhaps be casting function pointer?)
           */
          auto CI = dyn_cast<CallInst>(&I);
          for (auto i = 0U, end = CI->getNumArgOperands(); i < end; ++i) {
            auto paced = generatePACedValue(F.getParent(), I, CI->getArgOperand(i));
            if (paced != nullptr) {
              log->inc(DEBUG_TYPE ".PacFunctionArgument", true, F.getName()) << "PACing function argument\n";
              CI->setArgOperand(i, paced);
            }
          }

          // 2: Handle indirect function calls
          if (CallSite(CI).isIndirectCall()) {
            auto calledValue = CI->getCalledValue();
            const auto calledValueType = calledValue->getType();

            log->inc(DEBUG_TYPE ".AutIndirectCall", true, F.getName()) << "found indirect call!!!!\n";
            // Generate Builder for inserting pa_autia
            IRBuilder<> Builder(&I);
            // Get pa_autia declaration for correct input type
            auto autia = Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_autia, { calledValueType });
            // Get type_id as Constant
            auto typeIdConstant = PartsTypeMetadata::idConstantFromType(F.getContext(), calledValueType);
            // Insert intrinsics to authenticated the signed function pointer
            auto paced = Builder.CreateCall(autia, { calledValue, typeIdConstant }, "");

            // Replace signed pointer with the authenticated one
            CI->setCalledFunction(paced);
          }
          break;
        }
      }
    }
  }

  return true;
}

Value *PartsOptCpiPass::generatePACedValue(Module *M, Instruction &I, Value *V) {
  /*
   * We need to sign two types of function pointer arguments:
   *  1) a direct function
   *  2) a direct function passed via BitCastOperator
   *
   * In both cases we want to sign the resulting value based on the *source* type!
   * E.g., a function pointer bitcast to void* will be signed based on the source type.
   */
  auto VTypeInput = isa<BitCastOperator>(V)
      ? dyn_cast<BitCastOperator>(V)->getSrcTy()
      : V->getType();

  // We can directly skip if we don't need to do anything
  if (! VTypeInput->isPointerTy())
    return nullptr;

  // We need to inspect the source of the bitcast, otherwise the plain V
  auto VInput = isa<BitCastOperator>(V) ? dyn_cast<BitCastOperator>(V)->getOperand(0) : V;

  if (isa<Function>(VInput) && !dyn_cast<Function>(VInput)->isIntrinsic()) {
    // Get the type of the V, i.e,. the value we are going to sign
    const auto VType = V->getType();
    assert((isa<BitCastOperator>(V) || VType == VTypeInput) && "Vtype and VTypeInput should match unless bitcast");
    // Generate Builder for inserting pa_pacia
    IRBuilder<> Builder(&I);
    // Get pa_pacia declaration for correct type
    auto pacia = Intrinsic::getDeclaration(M, Intrinsic::pa_pacia, { VType });
    // Get type_id as Constant
    auto typeIdConstant = PartsTypeMetadata::idConstantFromType(M->getContext(), VType);
    // Insert intrinsics to generate PACed Value and return it
    return Builder.CreateCall(pacia, { V, typeIdConstant }, "");
  }

  return nullptr;
}
