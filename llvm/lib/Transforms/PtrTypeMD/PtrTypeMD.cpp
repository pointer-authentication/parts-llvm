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

#define DEBUG_TYPE "PtrTypeMDPass"
#define TAG KBLU DEBUG_TYPE ": "

namespace {

struct PtrTypeMDPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  PartsLog_ptr log;

  PtrTypeMDPass() :
      FunctionPass(ID),
      log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool runOnFunction(Function &F) override;

  PartsTypeMetadata_ptr createCallMetadata(Function &F, Instruction &I);
  PartsTypeMetadata_ptr createLoadMetadata(Function &F, Instruction &I);
  PartsTypeMetadata_ptr createStoreMetadata(Function &F, Instruction &I);

  void fixDirectFunctionArgs(Function &F, Instruction &I);
};

} // anonymous namespace

char PtrTypeMDPass::ID = 0;
static RegisterPass<PtrTypeMDPass> X("ptr-type-md-pass", "Pointer Type Metadata Pass");

bool PtrTypeMDPass::runOnFunction(Function &F) {
  if ( !(PARTS::useFeCfi() || PARTS::useDpi()))
    return false;

  DEBUG_PA(log->debug() << "Function: " << F.getName() << "\n");

  auto &C = F.getContext();

  for (auto &BB:F){
    DEBUG_PA(log->debug() << "  BasicBlock: " << BB.getName() << "\n");

    for (auto &I: BB) {
      DEBUG_PA(log->debug() << "  " << I << "\n");

      const auto IOpcode = I.getOpcode();

      PartsTypeMetadata_ptr MD = nullptr;

      switch(IOpcode) {
        case Instruction::Store:
          MD = createStoreMetadata(F, I);
          break;
        case Instruction::Load:
          MD = createLoadMetadata(F, I);
          break;
        case Instruction::Call:
          fixDirectFunctionArgs(F, I);
          MD = createCallMetadata(F, I);
          break;
        default:
          break;
      }

      if (MD != nullptr) {
        MD->attach(C, I);
        log->inc(DEBUG_TYPE ".MetadataAddedWithTypeId", !MD->isIgnored()) << "      adding metadata " << MD->toString() << "\n";
      } else {
        log->inc(DEBUG_TYPE ".MetadataNotAdded") << "      " << "skipping\n";
      }
    }
  }

  return true;
}

PartsTypeMetadata_ptr PtrTypeMDPass::createLoadMetadata(Function &F, Instruction &I) {
  assert(isa<LoadInst>(I));

  auto MD = PartsTypeMetadata::get(I.getType());

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

PartsTypeMetadata_ptr PtrTypeMDPass::createStoreMetadata(Function &F, Instruction &I) {
  assert(isa<StoreInst>(I));

  auto MD = PartsTypeMetadata::get(I.getOperand(0)->getType());

  if (MD->isCodePointer()) {
    // We need to ignore code-pointers, unless they are freshly "created" from a function
    auto *S = dyn_cast<StoreInst>(&I);
    // The function address is a constant
    const auto *constant = dyn_cast<Constant>(S->getValueOperand());
    // Also ignore NULL pointers, they are unusable anyway
    if (constant == nullptr || constant->isZeroValue()) {
      MD->setIgnored(true);
    }
  } else if (MD->isDataPointer()) {
    if (!PARTS::useDpi())
      MD->setIgnored(true);
  }

  return MD;
}

void PtrTypeMDPass::fixDirectFunctionArgs(Function &F, Instruction &I) {
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

PartsTypeMetadata_ptr PtrTypeMDPass::createCallMetadata(Function &F, Instruction &I) {
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
