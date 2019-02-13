//===----------------------------------------------------------------------===//
//
// Authors: Zaheer Ahmed Gauhar <zaheer.gauhar@aalto.fi>
//          Hans Liljestrand <hans.liljestrand@pm.me>
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

#define DEBUG_TYPE "PartsOptDpiPass"
#define TAG KBLU DEBUG_TYPE ": "

namespace {

class PartsOptDpiPass : public FunctionPass {
  public:
    static char ID; // Pass identification, replacement for typeid

    PartsLog_ptr log;

    PartsOptDpiPass() :
        FunctionPass(ID),
        log(PartsLog::getLogger(DEBUG_TYPE))
    {
      DEBUG_PA(log->enable());
    }

    bool runOnFunction(Function &F) override;

  private:
    PartsTypeMetadata_ptr createLoadMetadata(Function &F, Instruction &I);
    PartsTypeMetadata_ptr createStoreMetadata(Function &F, Instruction &I);
    inline bool handleInstruction(Function &F, Instruction &I);
};

} // anonymous namespace

char PartsOptDpiPass::ID = 0;
static RegisterPass<PartsOptDpiPass> X("parts-opt-dpi", "PARTS DPI pass");

bool PartsOptDpiPass::runOnFunction(Function &F) {

  if (!PARTS::useDpi())
    return false;

  bool function_modified = false;

  for (auto &BB:F)
    for (auto &I: BB)
      function_modified |= handleInstruction(F, I);

  return function_modified;
}

inline bool PartsOptDpiPass::handleInstruction(Function &F, Instruction &I)
{
  auto &C = F.getContext();

  DEBUG_PA(log->debug() << F.getName() << "->" << BB.getName() << "->" << I << "\n");

  const auto IOpcode = I.getOpcode();

  PartsTypeMetadata_ptr MD = nullptr;

  switch(IOpcode) {
    case Instruction::Store:
      MD = createStoreMetadata(F, I);
      break;
    case Instruction::Load:
      MD = createLoadMetadata(F, I);
      break;
    default:
      break;
  }

  if (MD != nullptr) {
    MD->attach(C, I);
    log->inc(DEBUG_TYPE ".MetadataAdded", !MD->isIgnored()) << "adding metadata: " << MD->toString() << "\n";
  } else {
    log->inc(DEBUG_TYPE ".MetadataMissing") << "missing metadata\n";
    return false;
  }
  return true;
}

PartsTypeMetadata_ptr PartsOptDpiPass::createLoadMetadata(Function &F, Instruction &I) {
  assert(isa<LoadInst>(I));

  auto V = I.getOperand(0);
  assert(I.getType() == V->getType()->getPointerElementType());

  PartsTypeMetadata_ptr MD = nullptr;

  if (isa<BitCastInst>(V)) {
    auto BC = dyn_cast<BitCastInst>(V);
    MD = PartsTypeMetadata::get(BC->getSrcTy());
    // FIXME: Ugly hack, will make all union types the same!!!
  } else {
    MD = PartsTypeMetadata::get(V->getType()->getPointerElementType());
  }

  if (MD->isCodePointer()) {
    // Ignore all loaded function-pointers (at least for now)
    MD->setIgnored(true);
  } else if (MD->isDataPointer()) {
    if (!PARTS::useDpi()) {
      MD->setIgnored(true);
    }
  }

  return MD;
}

PartsTypeMetadata_ptr PartsOptDpiPass::createStoreMetadata(Function &F, Instruction &I) {
  assert(isa<StoreInst>(I));

  auto V = I.getOperand(1);
  assert(I.getOperand(0)->getType() == V->getType()->getPointerElementType());

  PartsTypeMetadata_ptr MD = nullptr;

  if (isa<BitCastInst>(V)) {
    auto BC = dyn_cast<BitCastInst>(V);
    MD = PartsTypeMetadata::get(BC->getSrcTy());
    // FIXME: Ugly hack, will make all union types the same!!!
  } else {
    MD = PartsTypeMetadata::get(V->getType()->getPointerElementType());
  }

  if (MD->isCodePointer()) {
    MD->setIgnored(true);
  } else if (MD->isDataPointer()) {
    if (!PARTS::useDpi())
      MD->setIgnored(true);
  }

  return MD;
}
