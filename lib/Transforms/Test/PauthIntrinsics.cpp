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
#define TAG KCYN DEBUG_TYPE ": "

namespace {

struct PauthIntrinsics: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  PauthIntrinsics() : FunctionPass(ID) {}

  Module *Mptr;

  Function* pauth_pacda = nullptr;
  Function* pauth_autda = nullptr;

  bool doInitialization(Module &M) override;
  bool doFinalization(Module &M) override;

  bool pauthStore(Function &F, Instruction &I);

  bool runOnFunction(Function &F) override;
};

} // anonyous namespace

char PauthIntrinsics::ID = 0;
static RegisterPass<PauthIntrinsics> X("pauth-intrin", "use pauth intrinsics");

bool PauthIntrinsics::doInitialization(Module &M) {
  Mptr = &M;

  Type *Tys[] = { Type::getInt8PtrTy(M.getContext()), Type::getInt32Ty(M.getContext()) };
  pauth_pacda = Intrinsic::getDeclaration(&M, Intrinsic::aarch64_pacda, Tys);
  pauth_autda = Intrinsic::getDeclaration(&M, Intrinsic::aarch64_autda, Tys);
  assert(pauth_autda != nullptr);
  assert(pauth_pacda != nullptr);

  //errs() << TAG << "pacda expecting " << pauth_pacda->getFunctionType()->getNumParams() << " params\n";
  //errs() << TAG << "pacda expecting param of type ";
  //pauth_pacda->getFunctionType()->getParamType(0)->dump();
  //errs() << KNRM;

  return true;
}

bool PauthIntrinsics::doFinalization(Module &M) {
  return false;
}

bool PauthIntrinsics::runOnFunction(Function &F) {
  DEBUG_PA_OPT(&F, do { errs() << TAG << "Function: "; errs().write_escaped(F.getName()) << "\n"; } while(false));

  for (auto &BB:F){
    DEBUG_PA_OPT(&F, do { errs() << TAG << "\tBasicBlock: "; errs().write_escaped(BB.getName()) << '\n'; } while(false));

    for (auto &I: BB) {
      DEBUG_PA_OPT(&F, do { errs() << TAG << "\t\t"; I.dump(); } while(false));

      switch(I.getOpcode()) {
        default:
          break;
        case Instruction::Store:
          pauthStore(F, I);
          break;
      }
    }
  }

  return true;
}

bool PauthIntrinsics::pauthStore(Function &F, Instruction &I) {
  auto *ptrOp = I.getOperand(0);
  auto *ptrType = ptrOp->getType();

  //I.dump();

  //DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\tTrying " << IType << "\n");

  if (!ptrType->isPointerTy())
    return false;

  DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\tfound a pointer store\n");

  std::vector<Value*> args(0);
  args.push_back(ptrOp);

  Type *arg_types[] = { ptrType };

  DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\tgetting pacda declaration for args " << arg_types << "\n");
  auto pac = Intrinsic::getDeclaration(Mptr, Intrinsic::aarch64_pacda, arg_types);

  DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\tcreating pacda call with args " << ptrOp << "\n");
  IRBuilder<> Builder(&I);
  auto added_I = Builder.CreateCall(pauth_pacda, args, "PACDANAME");

  DEBUG_PA_OPT(&F, do { errs() << TAG << "\t\t\t *** added "; added_I->dump(); } while(false));

  return true;
}

