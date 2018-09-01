// PauthPacMain.cpp - Code for Pointer Authentication
//
//                     The LLVM Compiler Infrastructure
//
// This code is released under Apache 2.0 license.
// Author: Hans Liljestrand
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//

#include <llvm/IR/IRBuilder.h>
#include <llvm/PARTS/PartsTypeMetadata.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"

using namespace llvm;

#define DEBUG_TYPE "PauthPacMain"

namespace {

struct PauthPacMain: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  Function *funcFixMain = nullptr;

  PauthPacMain() : FunctionPass(ID) {}

  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;

  bool runOnFunction(Function &F) override;
};

} // anonyous namespace

char PauthPacMain::ID = 0;
static RegisterPass<PauthPacMain> X("pauth-pacmain", "PAC argv for main call");

bool PauthPacMain::doInitialization(Module &M)
{
  auto &C = M.getContext();

  Type* types[3];
  types[0] = Type::getInt32Ty(C);
  types[1] = PointerType::get(Type::getInt8PtrTy(C), 0);
  types[2] = Type::getInt64Ty(C);

  ArrayRef<Type*> params(types, 3);
  auto result = Type::getVoidTy(C);

  FunctionType* signature = FunctionType::get(result, params, false);
  funcFixMain = Function::Create(signature, Function::ExternalLinkage, "__pauth_pac_main_args", &M);

  return true;
}

bool PauthPacMain::doFinalization(Module &M) {
  return true;
}

bool PauthPacMain::runOnFunction(Function &F) {
  if (!F.getName().equals("main"))
    return false;

  assert(F.getName().equals("main"));

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

  errs() << "Adding call to __pauth_pac_main_args\n";
  return true;
}

