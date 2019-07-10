//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This pass just assigns unique function_ids to all functions in the module
//
//===----------------------------------------------------------------------===//

#include <set>
#include <iterator>
#include <llvm/IR/IRBuilder.h>
#include <llvm/PARTS/PartsTypeMetadata.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/RandomNumberGenerator.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsLog.h"

using namespace llvm;
using namespace llvm::PARTS;

#define DEBUG_TYPE "PartsOptRasPass"

namespace {

struct PartsOptRasPass: public ModulePass {
  static char ID;

  PartsLog_ptr log;

  PartsOptRasPass() :
      ModulePass(ID),
      log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool runOnModule(Module &M) override;
};

} // anonyous namespace

char PartsOptRasPass::ID = 0;
static RegisterPass<PartsOptRasPass> X("parts-opt-ras", "PARTS return address signing opt pass");

Pass *llvm::PARTS::createPartsOptRasPass() { return new PartsOptRasPass(); }

bool PartsOptRasPass::runOnModule(Module &M) {
  std::unique_ptr<RandomNumberGenerator> RNG = M.createRNG(this);

  /* A bit crude, but, use st to keep track of used numbers */
  std::set<uint64_t> used_numbers;

  assert(RNG->min() == 0 && RNG->max() == UINT64_MAX);

  for (auto &F : M) {
    uint64_t num;

    do { /* Find a random function_id that has not yet been assigned */
      num = (*RNG)();
    } while (used_numbers.count(num) != 0);

    /* This is also a bit crude, we're passing an uint64_t as a string */
    F.addFnAttr("parts-function_id", std::to_string(num));
  };

  return true;
}

