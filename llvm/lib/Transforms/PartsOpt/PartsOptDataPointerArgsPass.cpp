//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
// Copyright (C) 2019 Huawei Finland Oy
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/IRBuilder.h>
#include <llvm/PARTS/PartsTypeMetadata.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsLog.h"
#include "llvm/PARTS/PartsOptPass.h"

using namespace llvm;
using namespace llvm::PARTS::PartsOptPass;

#define DEBUG_TYPE "PartsOptDataPointerArgsPass"

namespace {

struct PartsOptDataPointerArgsPass: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  Function *funcFixMain = nullptr;

  PartsLog_ptr log;

  PartsOptDataPointerArgsPass() :
      FunctionPass(ID),
      log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool runOnFunction(Function &F) override;

private:
  bool insertIntrinsic(Function &F, Argument *A, Instruction &I);
};

} // anonyous namespace

// FIXME: We have a dependency to the DeadArgumentEliminationPass module pass.
// We have to run after DAE pass to avoid keeping dead data pointers arguments around inner functions.

char PartsOptDataPointerArgsPass::ID = 0;
static RegisterPass<PartsOptDataPointerArgsPass> X("parts-opt-dp-args", "PARTS mark data pointer function arguments");

bool PartsOptDataPointerArgsPass::runOnFunction(Function &F) {
  if (!(PARTS::useDpi()))
    return false;

  bool modified = false;
  auto &B = F.getEntryBlock();
  auto &I = *B.begin();

  for (auto AI = F.arg_begin(), AE = F.arg_end(); AI != AE; ++AI)
    if (isDataPointer(AI->getType()))
      modified = insertIntrinsic(F, AI, I);

  return modified;
}

bool PartsOptDataPointerArgsPass::insertIntrinsic(Function &F, Argument *A, Instruction &I) {
  auto dp_arg = createPartsIntrinsicNoTypeID(F, I, A,
                                       Intrinsic::parts_data_pointer_argument);
  A->replaceAllUsesWith(dp_arg);
  dp_arg->setOperand(0, A);

  return true;
}
