// PauthIntrinsics.cpp - Code for Pointer Authentication
//
//                     The LLVM Compiler Infrastructure
//
// This code is released under Apache 2.0 license.
// Author: Hans Liljestrand
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//

#include <llvm/IR/IRBuilder.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "../../Target/AArch64/PointerAuthentication.h"

using namespace llvm;

#define DEBUG_TYPE "PauthIntrinsics"
#define TAG KYEL DEBUG_TYPE ": "

namespace {

struct PauthIntrinsics: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  PauthIntrinsics() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;
};

} // anonyous namespace

char PauthIntrinsics::ID = 0;
static RegisterPass<PauthIntrinsics> X("pauth-intrin", "use pauth intrinsics");

bool PauthIntrinsics::runOnFunction(Function &F) {
  DEBUG_PA_OPT(&F, errs() << TAG << "Function: ");
  DEBUG_PA_OPT(&F, errs().write_escaped(F.getName()) << "\n");

  for (auto &BB:F){
    DEBUG_PA_OPT(&F, errs() << TAG << "\tBasicBlock: ");
    DEBUG_PA_OPT(&F, errs().write_escaped(BB.getName()) << '\n' << KNRM);

    for (auto &I: BB) {
      DEBUG_PA_OPT(&F, errs() << TAG << "\t\t");
      DEBUG_PA_OPT(&F, I.dump());

      const auto IOpcode = I.getOpcode();

      if (IOpcode == Instruction::Store) {
        const auto *IType = I.getOperand(0)->getType();

        if (IType->isPointerTy()) {
          DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\tfoudn a pointer store\n");

          /*
          Type* types[2];
          types[0] = Type::getInt32Ty(M.getContext());
          types[1] = PointerType::get(Type::getInt8PtrTy(M.getContext()), 0);
          ArrayRef<Type*> params(types, 2);
          auto result = Type::getVoidTy(M.getContext());

          FunctionType* signature = FunctionType::get(result, params, false);
          funcFixMain = Function::Create(signature, Function::ExternalLinkage, "__pauth_pac_main_args", &M);

          std::vector<Value*> args(0);
          args.push_back(&argc);
          args.push_back(&argv);

          IRBuilder<> Builder(&I);
          Builder.CreateCall(funcFixMain, args);
          */
        }
      }
    }
  }

  return true;
}

