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
#include "llvm/PARTS/PartsTypeMetadata.h"

using namespace llvm;
using namespace llvm::PARTS;

#define DEBUG_TYPE "PartsOptCpiPass"

STATISTIC(StatSignStoreFunction, DEBUG_TYPE ": Number of code pointers signed on store");
STATISTIC(StatSignFunctionArg, DEBUG_TYPE ": Number of code pointers signed when passed as arguments");
STATISTIC(StatAuthenticateIndirectCall, DEBUG_TYPE ": Number of code pointers authenticated on indirect call");

namespace {

class PartsOptCpiPass : public FunctionPass {
public:
  static char ID;

  PartsOptCpiPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

private:

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
  bool handleInstruction(Function &F, Instruction &I);
  bool handleCallInstruction(Function &F, Instruction &I);
  bool handleStoreInstruction(Function &F, Instruction &I);
  bool handleSelectInstruction(Function &F, Instruction &I);
};

} // anonymous namespace

char PartsOptCpiPass::ID = 0;
static RegisterPass<PartsOptCpiPass> X("parts-opt-cpi", "PARTS CPI pass");

bool PartsOptCpiPass::runOnFunction(Function &F) {
  if (!PARTS::useFeCfi())
    return false;

  bool modified = false;
  for (auto &BB:F)
    for (auto &I: BB)
      modified = handleInstruction(F, I) || modified;

  return modified;
}

bool PartsOptCpiPass::handleInstruction(Function &F, Instruction &I)
{
  const auto IOpcode = I.getOpcode();

  switch(IOpcode) {
    case Instruction::Store:
      return handleStoreInstruction(F, I);
      break;
    case Instruction::Select:
      return handleSelectInstruction(F, I);
      break;
    case Instruction::Call:
      return handleCallInstruction(F, I);
      break;
  }

  return false;
}

bool PartsOptCpiPass::handleStoreInstruction(Function &F, Instruction &I)
{
  auto SI = dyn_cast<StoreInst>(&I);
  assert(SI != nullptr && "this should always be a store instruction, maybe remove this assert?");

  auto paced = generatePACedValue(F.getParent(), I, SI->getValueOperand());
  if (paced == nullptr)
    return false;

  ++StatSignStoreFunction;
  SI->setOperand(0, paced);

  return true;
}

bool PartsOptCpiPass::handleSelectInstruction(Function &F, Instruction &I) {

  bool modified = false;

  for (unsigned i = 0, end = I.getNumOperands(); i < end; ++i) {
    auto paced = generatePACedValue(F.getParent(), I, I.getOperand(i));
    if (paced != nullptr) {
      modified = true;
      I.setOperand(i, paced);
    }
  }

  return modified;
}
bool PartsOptCpiPass::handleCallInstruction(Function &F, Instruction &I)
{
  /*
   * For functions we need to do two things:
   * 1: Either sign or unsign pointer args based on function linkage
   *        e.g., ~ func(printf)        -> func(pacia(printf)))
   *              ~ qsort(..., compare) -> qsort(..., compare)
   *              ~ qsort(..., ptr)     -> qsort(..., autia(compare))
   * 2: Make sure indirect function calls are authenticated.
   * FIXME: handle pointers InlineAsm
   * FIXME: handle indirect calls using BitCastOperator (could perhaps be casting function pointer?)
   */
  auto CI = dyn_cast<CallInst>(&I);
  auto CalledFunction = CI->getCalledFunction();

  if (CalledFunction != nullptr && CalledFunction->isDeclaration() &&
      !CalledFunction->hasFnAttribute("parts-cpi")) {
    // We assume externally linked function do not have Parts instrumentation, and must, not only
    // omit PACing of args, but also authenticate and unPAC ingoing signed pointers.
    //
    // NOTE: This cannot detect indirect calls to externally linked functions!
    for (auto i = 0U, end = CI->getNumArgOperands(); i < end; ++i) {
      auto arg = CI->getArgOperand(i);
      const auto argType = arg->getType();
      if (argType->isPointerTy() &&
          argType->getPointerElementType()->isFunctionTy() &&
          !isa<Constant>(arg)) {

        IRBuilder<> Builder(&I);
        auto authenticatedArg =  Builder.CreateCall(
            Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_autia, { argType }),
            { arg, PartsTypeMetadata::idConstantFromType(F.getContext(), argType) }
        );
        CI->setArgOperand(i, authenticatedArg);
      }
    }
  } else {
    // Make sure args are signed for non-externally linked functions
    for (auto i = 0U, end = CI->getNumArgOperands(); i < end; ++i) {
      auto paced = generatePACedValue(F.getParent(), I, CI->getArgOperand(i));
      if (paced != nullptr) {
        ++StatSignFunctionArg;
        CI->setArgOperand(i, paced);
      }
    }
  }

  /*
  const Value *V = getCalledValue();
  if (!V)
    return false;
  if (isa<FunTy>(V) || isa<Constant>(V))
    return false;
  if (const CallInst *CI = dyn_cast<CallInst>(getInstruction())) {
    if (CI->isInlineAsm())
      return false;
  }
  return true;
   */

  // 2: Handle indirect function calls
  if (CallSite(CI).isIndirectCall()) {
    auto calledValue = CI->getCalledValue();
    const auto calledValueType = calledValue->getType();

    ++StatAuthenticateIndirectCall;
    // Generate Builder for inserting pa_autcall
    IRBuilder<> Builder(&I);
    // Get pa_autia declaration for correct input type
    auto autcall = Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_autcall, { calledValueType });
    // Get type_id as Constant
    auto typeIdConstant = PartsTypeMetadata::idConstantFromType(F.getContext(), calledValueType);
    // Insert intrinsics to authenticated the signed function pointer
    auto paced = Builder.CreateCall(autcall, { calledValue, typeIdConstant }, "");

    // Replace signed pointer with the authenticated one
    CI->setCalledFunction(paced);
  }

  return true;
}

Value *PartsOptCpiPass::generatePACedValue(Module *M, Instruction &I, Value *V) {
  /*
   * We need to handle two types of function pointer arguments:
   *  1) a direct function
   *  2) a direct function passed via BitCastOperator
   *
   * In both cases we want to (un)sign the resulting value based on the *source* type!
   * E.g., a function pointer bitcast to void* will be (un)signed based on the source type.
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
