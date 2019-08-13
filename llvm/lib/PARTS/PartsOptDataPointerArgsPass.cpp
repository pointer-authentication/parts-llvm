//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
//         Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/IRBuilder.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsOptPass.h"

using namespace llvm;
using namespace llvm::PARTS::PartsOptPass;

#define DEBUG_TYPE "PartsOptDataPointerArgsPass"

namespace {

struct PartsOptDataPointerArgsPass: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  PartsOptDataPointerArgsPass() : FunctionPass(ID) {}
  bool runOnFunction(Function &F) override;

private:
  bool insertIntrinsic(Function &F, Value *A, Instruction &I);
  bool handleInstruction(Function &F, Instruction &I);
  bool isPartsIntrinsic(Instruction &I);
  bool markDataPointerArguments(Function &F);
  bool markDataPointerCallReturns(Function &F);
};

} // anonyous namespace

// FIXME: We have a dependency to the DeadArgumentEliminationPass module pass.
// We have to run after DAE pass to avoid keeping dead data pointers arguments around inner functions.

char PartsOptDataPointerArgsPass::ID = 0;
static RegisterPass<PartsOptDataPointerArgsPass> X("parts-opt-dp-args", "PARTS mark data pointer function arguments");

Pass *llvm::PARTS::createPartsOptDataPointerArgsPass() { return new PartsOptDataPointerArgsPass(); }

bool PartsOptDataPointerArgsPass::runOnFunction(Function &F) {
  if (F.hasFnAttribute("no-parts")) return false;
  if (!(PARTS::useDpi()))
    return false;

  bool modified = markDataPointerArguments(F);
  return markDataPointerCallReturns(F) || modified;
}

bool PartsOptDataPointerArgsPass::markDataPointerArguments(Function &F) {
  bool modified = false;
  auto &B = F.getEntryBlock();
  auto &I = *B.begin();

  for (auto AI = F.arg_begin(), AE = F.arg_end(); AI != AE; ++AI)
    if (isDataPointer(AI->getType()))
      modified = insertIntrinsic(F, AI, I);

  return modified;
}

bool PartsOptDataPointerArgsPass::markDataPointerCallReturns(Function &F) {
  bool modified = false;

  for (auto &BB:F)
    for (auto &I: BB)
      modified = handleInstruction(F, I) || modified;

  return modified;
}

inline bool PartsOptDataPointerArgsPass::handleInstruction(Function &F, Instruction &I) {
  bool modified = false;
  switch(I.getOpcode()) {
    default:
      break;
    case Instruction::Call:
      if (isPartsIntrinsic(I)) break;
      LLVM_FALLTHROUGH;
    case Instruction::Select:
    case Instruction::PHI:
    case Instruction::BitCast:
    case Instruction::GetElementPtr:
      if (isDataPointer(I.getType()))
        modified = insertIntrinsic(F, &I, *I.getNextNode());
      break;
  }

  return modified;
}

inline bool PartsOptDataPointerArgsPass::isPartsIntrinsic(Instruction &I) {
  CallInst *CI = dyn_cast<CallInst>(&I);
  Function *CalledFunc = CI->getCalledFunction();
  // FIXME: Check PARTS Instrisic Opcode. We currently ignore ALL intrisics. Is this correct ?
  return CalledFunc && CalledFunc->isIntrinsic();
}

bool PartsOptDataPointerArgsPass::insertIntrinsic(Function &F, Value *A, Instruction &I) {
  auto dp_arg = createPartsIntrinsicNoTypeID(F, I, A,
                                       Intrinsic::parts_data_pointer_argument);
  dp_arg->setOperand(0, A);

  return true;
}
