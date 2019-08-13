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
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsIntr.h"

using namespace llvm;
using namespace llvm::PARTS;

#define DEBUG_TYPE "PartsOptGlobalsPass"

namespace {

STATISTIC(PartsDataPointersPACed, "global data pointers to PAC");
STATISTIC(PartsCodePointersPACed, "global code pointers to PAC");

struct PartsOptGlobalsPass: public ModulePass {
  static char ID;

  PartsOptGlobalsPass() : ModulePass(ID) {}

  bool runOnModule(Module &M) override;

private:
  IRBuilder<> *builder;

  bool handle(Module &M, Value *V, Constant *CV, Type *Ty);
  bool handle(Module &M, Value *V, Constant *CV, StructType *Ty);
  bool handle(Module &M, Value *V, Constant *CV, ArrayType *Ty);
  bool handle(Module &M, Value *V, Constant *CV, PointerType *Ty);
  bool needsPACing(Constant *CV, PointerType *Ty);
  void updateStatistics(PointerType *Ty);
};

} // anonymous namespace

char PartsOptGlobalsPass::ID = 0;
static RegisterPass<PartsOptGlobalsPass> X("parts-opt-globals", "PARTS globals fix, needed for CPI and DPI");

Pass *llvm::PARTS::createPartsOptGlobalsPass() { return new PartsOptGlobalsPass(); }

bool PartsOptGlobalsPass::runOnModule(Module &M) {
  if ( !(PARTS::useFeCfi() || PARTS::useDpi())) // We don't need to do anything unless we use PI
    return false;

  auto &C = M.getContext();

  auto result = Type::getVoidTy(C);
  FunctionType* signature = FunctionType::get(result, false);
  Function *funcFixGlobals = Function::Create(signature, Function::PrivateLinkage, "__pauth_pac_globals", &M);
  funcFixGlobals->addFnAttr("no-parts", "true");
  funcFixGlobals->addFnAttr("noinline", "true");

  auto BB = BasicBlock::Create(M.getContext(), "entry", funcFixGlobals);
  IRBuilder<> localBuilder(BB);
  builder = &localBuilder;

  for (auto &GI: M.globals()) {
    if (GI.hasInitializer()) {
      const auto CV = GI.getInitializer();
      if (handle(M, &GI, CV, CV->getType())) {
        // We are going to modify the global, make sure it is writeable!
        if (GI.isConstant())
          GI.setConstant(false);
      }
    }

  }
  builder->CreateRetVoid();
  builder = nullptr;

  appendToGlobalCtors(M, funcFixGlobals, 0);

  return true;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, Constant *CV, Type *Ty) {
  if (Ty->isArrayTy())
    return handle(M, V, CV, dyn_cast<ArrayType>(Ty));

  if (Ty->isStructTy())
    return handle(M, V, CV, dyn_cast<StructType>(Ty));

  if (Ty->isPointerTy())
    return handle(M, V, CV, dyn_cast<PointerType>(Ty));

  return false;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, Constant *CV, ArrayType *Ty) {
  bool changed = false;
  auto &C = M.getContext();

  const auto elCount = Ty->getNumElements();

  uint64_t base = 0; // Store the based pointer for GEP instruction

  if (isa<GetElementPtrInst>(V))  // Get prior base if mult-level array
      base = dyn_cast<ConstantInt>(dyn_cast<User>(V)->getOperand(1))->getLimitedValue();

  // Handle pointer elements
  for (auto i = 0U; i < elCount; i++) {
    auto elPtr = builder->CreateGEP(V, {
        ConstantInt::get(Type::getInt64Ty(C), 0),
        ConstantInt::get(Type::getInt64Ty(C), base + i),
    });
    auto elCV = CV->getAggregateElement(base + i);
    changed = handle(M, elPtr, elCV, Ty->getElementType()) || changed;
  }

  return changed;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, Constant *CV, StructType *Ty) {
  bool changed = false;

  for (auto i = 0U; i < Ty->getNumElements(); i++) {
    auto elPtr = builder->CreateStructGEP(Ty, V, i);
    auto elType = Ty->getElementType(i);
    auto elCV = CV->getAggregateElement(i);
    changed = handle(M, elPtr, elCV, elType) || changed;
  }

  return changed;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, Constant *CV, PointerType *Ty) {

  if (!needsPACing(CV, Ty))
    return false;

  updateStatistics(Ty);
  auto loaded = builder->CreateLoad(V);
  auto paced = PartsIntr::pac_pointer(builder, M, loaded);
  builder->CreateStore(paced, V);

  return true;
}

bool PartsOptGlobalsPass::needsPACing(Constant *CV, PointerType *Ty) {

  const bool isCodePointer = Ty->getPointerElementType()->isFunctionTy();

  if (isCodePointer && PARTS::useFeCfi() && !CV->isNullValue())
      return true;
  else if (!isCodePointer && PARTS::useDpi())
    return true;

  return false;
}

void PartsOptGlobalsPass::updateStatistics(PointerType *Ty) {
  const bool isCodePointer = Ty->getPointerElementType()->isFunctionTy();

  if (isCodePointer)
    ++PartsCodePointersPACed;
 else
    ++PartsDataPointersPACed;
}
