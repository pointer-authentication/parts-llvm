//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
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

using namespace llvm;

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
};

} // anonyous namespace

// FIXME: We have a dependency to the DeadArgumentEliminationPass module pass.
// We have to run after DAE pass to avoid keeping dead data pointers arguments around inner functions.

char PartsOptDataPointerArgsPass::ID = 0;
static RegisterPass<PartsOptDataPointerArgsPass> X("parts-opt-dp-args", "PARTS mark data pointer function arguments");

bool PartsOptDataPointerArgsPass::runOnFunction(Function &F) {
  if (!(PARTS::useDpi()))
    return false;

#if 0
  auto AI = F.arg_begin();
  if (AI == F.arg_end())
    return false;

  if (AI->getType()->getTypeID() != Type::IntegerTyID)
    llvm_unreachable("first argument to main is not an integer!?!");

  auto &argc = *AI++;
  if (AI == F.arg_end() || AI->getType()->getTypeID() != Type::PointerTyID)
    llvm_unreachable("second argument to main not a char **ptr!?!\n");

  auto &argv = *AI++;
  if (AI != F.arg_end())
    llvm_unreachable("unexpected arguments to main!?!\n");

  auto &B = F.getEntryBlock();
  auto &I = *B.begin();

  std::vector<Value*> args(0);
  args.push_back(&argc);
  args.push_back(&argv);

  args.push_back(PartsTypeMetadata::idConstantFromType(
      F.getContext(),
      dyn_cast<PointerType>(argv.getType())->getElementType()
  ));

  IRBuilder<> Builder(&I);
  Builder.CreateCall(funcFixMain, args);
  return true;
#else
  return false;
#endif
}
