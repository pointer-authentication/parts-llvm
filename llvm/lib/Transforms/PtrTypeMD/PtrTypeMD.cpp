//===- PtrTypeMD.cpp - Code For Pointer Type Metadata ---------------------===/
//
//                     The LLVM Compiler Infrastructure
//
// This code is released under Apache 2.0 license.
// Author: Zaheer Ahmed Gauhar
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
//===----------------------------------------------------------------------===//
//
// This file implements Pointer type extraction for load and store 
// instructions and differentiates between pointers to data and functions.
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/PartsTypeMetadata.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Constant.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "MDclass.h"
#include "../../Target/AArch64/PointerAuthentication.h"

using namespace llvm;
using namespace llvm::PA;

#define DEBUG_TYPE "PtrTypeMDPass"
#define TAG KBLU DEBUG_TYPE ": "

namespace {

struct PtrTypeMDPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  MDclass md=MDclass();
  PtrTypeMDPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override;

  PartsTypeMetadata_ptr createCallMetadata(Function &F, Instruction &I);
  PartsTypeMetadata_ptr createLoadMetadata(Function &F, Instruction &I);
  PartsTypeMetadata_ptr createStoreMetadata(Function &F, Instruction &I);
};

} // anonyous namespace

char PtrTypeMDPass::ID = 0;
static RegisterPass<PtrTypeMDPass> X("ptr-type-md-pass", "Pointer Type Metadata Pass");

bool PtrTypeMDPass::runOnFunction(Function &F) {
  DEBUG_PA_OPT(&F, do { errs() << TAG << "Function: "; errs().write_escaped(F.getName()) << "\n"; } while(false));

  auto &C = F.getContext();

  for (auto &BB:F){
    DEBUG_PA_OPT(&F, do { errs() << TAG << "\tBasicBlock: "; errs().write_escaped(BB.getName()) << '\n'; } while(false));

    for (auto &I: BB) {
      DEBUG_PA_OPT(&F, do { errs() << TAG << "\t\t"; I.dump(); } while(false));

      const auto IOpcode = I.getOpcode();

      PartsTypeMetadata_ptr MD = nullptr;

      switch(IOpcode) {
        case Instruction::Store:
          MD = createStoreMetadata(F, I);
          break;
        case Instruction::Load:
          MD = createLoadMetadata(F, I);
          break;
        case Instruction::Call:
          MD = createCallMetadata(F, I);
          break;
        default:
          break;
      }

      if (MD != nullptr) {
        MD->attach(C, I);

        DEBUG_PA_OPT(&F, do {
          if (MD->isIgnored()) {
            DEBUG_PA_OPT(&F, errs() << TAG << KCYN << "\t\t\t adding metadata to ignore\n");
          } else {
            DEBUG_PA_OPT(&F, errs() << TAG << KGRN << "\t\t\t adding metadata (type_id=" << MD->getTypeId() << ")\n");
          }
        } while(0));
      } else {
        DEBUG_PA_OPT(&F, errs() << TAG << "\t\t\t skipping\n");
      }
    }
  }

  return true;
}

PartsTypeMetadata_ptr PtrTypeMDPass::createLoadMetadata(Function &F, Instruction &I) {
  assert(isa<LoadInst>(I));

  auto MD = PartsTypeMetadata::get(I.getType());

  if (MD->isCodePointer()) {
    // Ignore all loaded function-pointers (at least for now)
    MD->setIgnored(true);
  }

  return MD;
}

PartsTypeMetadata_ptr PtrTypeMDPass::createStoreMetadata(Function &F, Instruction &I) {
  assert(isa<StoreInst>(I));

  auto MD = PartsTypeMetadata::get(I.getOperand(0)->getType());

  if (MD->isCodePointer()) {
    // We need to ignore code-pointers, unless they are freshly "created" from a function
    auto *S = dyn_cast<StoreInst>(&I);
    // The function address is a constant
    const auto *constant = dyn_cast<Constant>(S->getValueOperand());
    // Also ignore NULL pointers, they are unusable anyway
    if (constant == nullptr || constant->isZeroValue()) {
      MD->setIgnored(true);
    }
  }

  return MD;
}

PartsTypeMetadata_ptr PtrTypeMDPass::createCallMetadata(Function &F, Instruction &I) {
  assert(isa<CallInst>(I));

  PartsTypeMetadata_ptr MD;

  const CallInst *CI = dyn_cast<CallInst>(&I);
  if (CI->getCalledFunction() == nullptr) {
    DEBUG_PA_OPT(&F, errs() << TAG << KGRN << "\t\t\t found indirect call!!!!\n");
    MD = PartsTypeMetadata::get(I.getOperand(0)->getType());
  } else {
    MD = PartsTypeMetadata::getIgnored();
  }

  errs() << "op num: " << CI->getNumOperands() << "\n";

  CI->dump();

  // Look at function arguments to see if we need to fix any of them
  for (auto i = 0U; i < CI->getNumOperands()-1; i++) {
    auto O =CI->getOperand(i);

    auto V = dyn_cast<Value>(O);
    V->getType()->dump();

    auto argMD = PartsTypeMetadata::get(V->getType());

    if (!(argMD->isCodePointer() || isa<GlobalValue>(O))) {
      continue;
    }

    // lets replace the argument with an PACed pointer here!!!!
    llvm_unreachable("unimplemented: no support yet for adding PACs at IR level, in this case for function args");
    // FIXME: need to imlement this with intrinsics that allow us to create a PACed input pointer for the function
    // Note: in this specific case this is quite acceptable because we are plan to have just a single
    // "pointer creation" for code pointers.
  }

  return MD;
}
