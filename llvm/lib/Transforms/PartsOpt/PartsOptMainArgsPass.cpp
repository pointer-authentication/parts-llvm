//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "PartsOptMainArgsPass"

namespace {

struct PartsOptMainArgsPass: public ModulePass {
  static char ID; // Pass identification, replacement for typeid

  PartsOptMainArgsPass() : ModulePass(ID) {}

  Function *createFixFunction(Module &M);
  bool runOnModule(Module &M) override;
};

} // anonymous namespace

char PartsOptMainArgsPass::ID = 0;
static RegisterPass<PartsOptMainArgsPass> X("parts-opt-mainargs",
                                            "PARTS DPI fix for main args");

Function *PartsOptMainArgsPass::createFixFunction(Module &M)
{
  assert(PARTS::useDpi());

  auto &C = M.getContext();

  Type* types[3];
  types[0] = Type::getInt32Ty(C);
  types[1] = PointerType::get(Type::getInt8PtrTy(C), 0);
  types[2] = Type::getInt64Ty(C);

  ArrayRef<Type*> params(types, 3);
  auto result = Type::getVoidTy(C);

  FunctionType* signature = FunctionType::get(result, params, false);
  Function *funcFixMain = Function::Create(signature, Function::InternalLinkage,
                                 "__pauth_pac_main_args", &M);

  auto *entry = BasicBlock::Create(M.getContext(), "entry", funcFixMain);

  IRBuilder<> Builder(entry);

  Builder.CreateRetVoid();
  return funcFixMain;
}

bool PartsOptMainArgsPass::runOnModule(Module &M) {
  if (!PARTS::useDpi())
    return false;

  auto *F = M.getFunction("main");

  auto AI = F->arg_begin();
  if (AI == F->arg_end())
    return false;

  auto &argc = *AI++;
  if (AI == F->arg_end())
    return false;

  auto &argv = *AI++;

  if (AI != F->arg_end())
    // Seems we have a main(int argc, char **argv, char **envp) type main
    llvm_unreachable("envp support not implemented!\n");

  assert(argc.getType()->getTypeID() == Type::IntegerTyID &&
         "first argument to main is not an integer!?!");
  assert(argv.getType()->getTypeID() == Type::PointerTyID &&
         "second argument to main not a char **ptr!?!");

  auto *funcFixMain = createFixFunction(M);

  auto &B = F->getEntryBlock();
  auto &I = *B.begin();

  std::vector<Value*> args(0);
  args.push_back(&argc);
  args.push_back(&argv);

  args.push_back(PartsTypeMetadata::idConstantFromType(
      M.getContext(),
      dyn_cast<PointerType>(argv.getType())->getElementType()
  ));

  IRBuilder<> Builder(&I);
  Builder.CreateCall(funcFixMain, args);

  DEBUG(dbgs() << "Adding call to __pauth_pac_main_args");
  return true;
}

