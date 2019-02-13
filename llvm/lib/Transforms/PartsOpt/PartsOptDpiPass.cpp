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
    inline bool handleInstruction(Function &F, Instruction &I);
    PartsTypeMetadata_ptr createLoadMetadata(Instruction &I);
    PartsTypeMetadata_ptr createStoreMetadata(Instruction &I);
    inline bool isLoadOrStore(const unsigned IOpcode);
    PartsTypeMetadata_ptr createMetadata(Value *V);
};

} // anonymous namespace

char PartsOptDpiPass::ID = 0;
static RegisterPass<PartsOptDpiPass> X("parts-opt-dpi", "PARTS DPI pass");

bool PartsOptDpiPass::runOnFunction(Function &F) {

  if (!PARTS::useDpi())
    return false;

  bool function_modified = false;

  for (auto &BB:F)
    for (auto &I: BB) {
      DEBUG_PA(log->debug() << F.getName() << "->" << BB.getName() << "->" << I << "\n");
      function_modified = handleInstruction(F, I) || function_modified;
  }

  return function_modified;
}

inline bool PartsOptDpiPass::handleInstruction(Function &F, Instruction &I)
{
  const auto IOpcode = I.getOpcode();

  if (!isLoadOrStore(IOpcode))
    return false;

  PartsTypeMetadata_ptr MD;

  if (IOpcode == Instruction::Store)
      MD = createStoreMetadata(I);
  else
      MD = createLoadMetadata(I);

  MD->attach(F.getContext(), I);
  log->inc(DEBUG_TYPE ".MetadataAdded", !MD->isIgnored()) << "adding metadata: " << MD->toString() << "\n";

  return true;
}

inline bool PartsOptDpiPass::isLoadOrStore(const unsigned IOpcode)
{
  return IOpcode == Instruction::Load || IOpcode ==Instruction::Store;
}

PartsTypeMetadata_ptr PartsOptDpiPass::createLoadMetadata(Instruction &I) {
  assert(isa<LoadInst>(I));
  auto V = I.getOperand(0);
  assert(I.getType() == V->getType()->getPointerElementType());

  return createMetadata(V);
}

PartsTypeMetadata_ptr PartsOptDpiPass::createStoreMetadata(Instruction &I) {
  assert(isa<StoreInst>(I));
  auto V = I.getOperand(1);
  assert(I.getOperand(0)->getType() == V->getType()->getPointerElementType());

  return createMetadata(V);
}

PartsTypeMetadata_ptr PartsOptDpiPass::createMetadata(Value *V) {
  PartsTypeMetadata_ptr MD;

  if (isa<BitCastInst>(V)) {
    auto BC = dyn_cast<BitCastInst>(V);
    MD = PartsTypeMetadata::get(BC->getSrcTy());
    // FIXME: Ugly hack, will make all union types the same!!!
  } else {
    MD = PartsTypeMetadata::get(V->getType()->getPointerElementType());
  }

  if (MD->isCodePointer())
    MD->setIgnored(true);

  return MD;
}
