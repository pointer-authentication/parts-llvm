//===----------------------------------------------------------------------===//
//
// Authors: Zaheer Ahmed Gauhar <zaheer.gauhar@aalto.fi>
//          Hans Liljestrand <hans@liljestrand.dev>
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
#include "llvm/IR/Constant.h"
#include "llvm/Pass.h"
#include "llvm/PARTS/Parts.h"
#include <llvm/PARTS/PartsOptPass.h>

using namespace llvm;
using namespace llvm::PARTS;
using namespace llvm::PARTS::PartsOptPass;

#define DEBUG_TYPE "PartsOptDpiPass"

STATISTIC(StatSignStoreData, "data pointer stores instrumented");
STATISTIC(StatAuthLoadData, "data pointer loads instrumented");

namespace {

class PartsOptDpiPass : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  PartsOptDpiPass() : FunctionPass(ID) {}
  bool runOnFunction(Function &F) override;

private:
  inline bool handleInstruction(Function &F, Instruction &I);
  bool handleStoreInstruction(Function &F, StoreInst *pSI);
  bool handleLoadInstruction(Function &F, LoadInst *pLI);
};

} // anonymous namespace

char PartsOptDpiPass::ID = 0;
static RegisterPass<PartsOptDpiPass> X("parts-opt-dpi", "PARTS DPI pass");

Pass *llvm::PARTS::createPartsOptDpiPass() { return new PartsOptDpiPass(); }

bool PartsOptDpiPass::runOnFunction(Function &F) {
  if (F.hasFnAttribute("no-parts")) return false;
  if (!PARTS::useDpi())
    return false;

  bool function_modified = false;

  for (auto &BB:F)
    for (auto &I: BB) {
      function_modified = handleInstruction(F, I) || function_modified;
    }

  return function_modified;
}

inline bool PartsOptDpiPass::handleInstruction(Function &F, Instruction &I) {
  bool modified = false;
  switch(I.getOpcode()) {
    default:
      break;
    case Instruction::Store:
      modified = handleStoreInstruction(F, dyn_cast<StoreInst>(&I));
      break;
    case Instruction::Load:
      modified = handleLoadInstruction(F, dyn_cast<LoadInst>(&I));
      break;
  }

  return modified;
}

bool PartsOptDpiPass::handleStoreInstruction(Function &F, StoreInst *pSI) {
  const auto V = pSI->getValueOperand();
  const auto VType = V->getType();

  if (! isDataPointer(VType))
    return false;

  pSI->setOperand(0, createPartsIntrinsic(F, *pSI, V, Intrinsic::pa_pacda));

  ++StatSignStoreData;
  return true;
}

bool PartsOptDpiPass::handleLoadInstruction(Function &F, LoadInst *pLI) {
  const auto VType = pLI->getPointerOperandType()->getPointerElementType();

  if (! isDataPointer(VType))
    return false;

  CallInst *authenticated = nullptr;

  if (isUnionTypePunningSupported() && isUnionMemberLoad(pLI))
    authenticated = createPartsIntrinsicNoTypeID(F, *pLI->getNextNode(), pLI, Intrinsic::pa_xpacd);
  else
    authenticated = createPartsIntrinsic(F, *pLI->getNextNode(), pLI, Intrinsic::pa_autda);
  assert(authenticated != nullptr);

  pLI->replaceAllUsesWith(authenticated);
  authenticated->setOperand(0, pLI);

  ++StatAuthLoadData;
  return true;
}
