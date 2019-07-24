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
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsOptPass.h"

using namespace llvm;
using namespace llvm::PARTS::PartsOptPass;

#define DEBUG_TYPE "PartsOptBuiltinsPass"

namespace {

struct PartsOptBuiltinsPass: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  PartsOptBuiltinsPass() : FunctionPass(ID) {}
  bool runOnFunction(Function &F) override;
private:
  bool isPartsBuiltin(Instruction &I);
};

} // anonyous namespace

char PartsOptBuiltinsPass::ID = 0;
static RegisterPass<PartsOptBuiltinsPass> X("parts-opt-builtins", "PARTS mark data pointer function arguments");

Pass *llvm::PARTS::createPartsOptBuiltinsPass() { return new PartsOptBuiltinsPass(); }

bool PartsOptBuiltinsPass::runOnFunction(Function &F) {
  bool modified = false;

  for (auto &BB:F)
    for (auto &I: BB)
      if (isPartsBuiltin(I))
            errs() << "I have found pa_modifier builtin \n";

  return modified;
}

inline bool PartsOptBuiltinsPass::isPartsBuiltin(Instruction &I) {
  if (I.getOpcode() != Instruction::Call)
    return false;

  CallInst *CI = dyn_cast<CallInst>(&I);
  Function *CalledFunc = CI->getCalledFunction();

  return CalledFunc && (CalledFunc->getIntrinsicID() == Intrinsic::pa_modifier);
}
