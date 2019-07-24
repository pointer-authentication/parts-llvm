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
};

} // anonyous namespace

char PartsOptBuiltinsPass::ID = 0;
static RegisterPass<PartsOptBuiltinsPass> X("parts-opt-builtins", "PARTS mark data pointer function arguments");

Pass *llvm::PARTS::createPartsOptBuiltinsPass() { return new PartsOptBuiltinsPass(); }

bool PartsOptBuiltinsPass::runOnFunction(Function &F) {
  return false;
}
