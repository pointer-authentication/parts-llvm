//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::PARTS;

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

Pass *llvm::PARTS::createPartsOptMainArgsPass() { return new PartsOptMainArgsPass(); }

Function *PartsOptMainArgsPass::createFixFunction(Module &M)
{
  assert(PARTS::useDpi());

  auto &C = M.getContext();
  auto *CTy = Type::getInt32Ty(C);

  Type* types[3];
  types[0] = CTy;
  types[1] = PointerType::get(Type::getInt8PtrTy(C), 0);

  ArrayRef<Type*> params(types, 2);
  auto result = Type::getVoidTy(C);

  FunctionType* signature = FunctionType::get(result, params, false);
  Function *funcFixMain = Function::Create(signature, Function::InternalLinkage,
                                 "__pauth_pac_main_args", &M);

  // Make sure we don't double PAC this function
  funcFixMain->addFnAttr("no-parts", "true");

  // Get the function arguments
  auto argsI = funcFixMain->arg_begin();
  Value &argc = *argsI++;
  Value &argv = *argsI;

  // We're going to create three BBs and corresponding builders
  auto *entry = BasicBlock::Create(M.getContext(), "entry", funcFixMain);
  auto *body = BasicBlock::Create(M.getContext(), "body", funcFixMain);
  auto *exit = BasicBlock::Create(M.getContext(), "exit", funcFixMain);
  IRBuilder<> entryBuilder(entry);
  IRBuilder<> bodyBuilder(body);
  IRBuilder<> exitBuilder(exit);

  // Entry block simply either returns or enters the loop
  auto *cmp = entryBuilder.CreateICmpSGT(&argc, ConstantInt::get(CTy, 0));
  entryBuilder.CreateCondBr(cmp, body, exit);

  // The loop conditional / index is update with a PHI node
  auto *cond = bodyBuilder.CreatePHI(Type::getInt32Ty(C), 2);
  cond->addIncoming(ConstantInt::get(Type::getInt32Ty(C), 0), entry);

  // Load a argv element
  auto *idx = bodyBuilder.CreateGEP(&argv, cond);
  auto *load = bodyBuilder.CreateLoad(idx);

  // PAC it
  auto paced = bodyBuilder.CreateCall(
      Intrinsic::getDeclaration(&M, Intrinsic::pa_pacda, { load->getType() }),
      {
          load,
          PARTS::getTypeIDConstantFrom(*load->getType(), M.getContext())
      }
  );

  // And store it back
  bodyBuilder.CreateStore(paced, idx);

  // Finally increment conditional and check exit condition
  auto *next = bodyBuilder.CreateAdd(cond, ConstantInt::get(CTy, 1));
  auto *exitCond = bodyBuilder.CreateICmpEQ(next, &argc);
  bodyBuilder.CreateCondBr(exitCond, exit, body);
  // Also add the update value to the cond PHI node
  cond->addIncoming(next, body);

  // The exit node just returns...
  exitBuilder.CreateRetVoid();

  return funcFixMain;
}

bool PartsOptMainArgsPass::runOnModule(Module &M) {
  if (!PARTS::useDpi())
    return false;

  auto *F = M.getFunction("main");

  if (F == nullptr || F->hasFnAttribute("no-parts")) return false;

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

  IRBuilder<> Builder(&I);
  Builder.CreateCall(funcFixMain, args);

  LLVM_DEBUG(dbgs() << "Adding call to __pauth_pac_main_args");
  return true;
}

