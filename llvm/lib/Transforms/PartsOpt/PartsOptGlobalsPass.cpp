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
#include <llvm/PARTS/PartsIntr.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include <llvm/Transforms/Utils/ModuleUtils.h>

using namespace llvm;

#define DEBUG_TYPE "PartsOptGlobalsPass"

namespace {

STATISTIC(PartsDataPointersPACed, "global data pointers to PAC");
STATISTIC(PartsCodePointersPACed, "global code pointers to PAC");

struct PartsOptGlobalsPass: public ModulePass {
  static char ID;

  PartsOptGlobalsPass() : ModulePass(ID) {}

  bool runOnModule(Module &M) override;

private:
  std::list<PARTS::type_id_t> data_type_ids = std::list<PARTS::type_id_t>(0);
  std::list<PARTS::type_id_t> code_type_ids = std::list<PARTS::type_id_t>(0);

  IRBuilder<> *builder;

  bool handle(Module &M, Value *V, Type *Ty);
  bool handle(Module &M, Value *V, StructType *Ty);
  bool handle(Module &M, Value *V, ArrayType *Ty);
  bool handle(Module &M, Value *V, PointerType *Ty);

  void writeTypeIds(Module &M, std::list<PARTS::type_id_t> &type_ids, const char *sectionName);

};

} // anonymous namespace

char PartsOptGlobalsPass::ID = 0;
static RegisterPass<PartsOptGlobalsPass> X("parts-opt-globals", "PARTS globals fix, needed for CPI and DPI");

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

  for (auto GI = M.global_begin(), GE = M.global_end(); GI != GE; ++GI) {

    if (GI->getNumOperands() > 0) {
      const auto O = GI->getOperand(0);
      handle(M, &*GI, O->getType());
    }

  }
  builder->CreateRetVoid();
  builder = nullptr;

  appendToGlobalCtors(M, funcFixGlobals, 0);

  if (PARTS::useFeCfi()) {
    writeTypeIds(M, code_type_ids, ".code_type_id");
  }

  if (PARTS::useDpi()) {
    writeTypeIds(M, data_type_ids, ".data_type_id");
  }

  return true;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, Type *Ty) {
  if (Ty->isArrayTy())
    return handle(M, V, dyn_cast<ArrayType>(Ty));

  if (Ty->isStructTy())
    return handle(M, V, dyn_cast<StructType>(Ty));

  if (Ty->isPointerTy())
    return handle(M, V, dyn_cast<PointerType>(Ty));

  return false;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, ArrayType *Ty) {
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

    changed = handle(M, elPtr, elPtr->getType()->getPointerElementType()) || changed;
  }

  return changed;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, StructType *Ty) {
  bool changed = false;

  for (auto i = 0U; i < Ty->getNumElements(); i++) {
    auto elPtr = builder->CreateStructGEP(Ty, V, i);
    auto elType = Ty->getElementType(i);

    changed = handle(M, elPtr, elType) || changed;
  }

  return changed;
}

bool PartsOptGlobalsPass::handle(Module &M, Value *V, PointerType *Ty) {
  auto PTMD = PartsTypeMetadata::get(Ty);

  if (PTMD->isCodePointer()) {
    if (PARTS::useFeCfi()) {
      ++PartsCodePointersPACed;
    } else {
      PTMD->setIgnored(true);
    }
  } else {
    assert(PTMD->isDataPointer());
    if (PARTS::useDpi()) {
      ++PartsDataPointersPACed;
    } else {
      PTMD->setIgnored(true);
    }
  }

  if (PTMD->isIgnored())
    return false;

  auto loaded = builder->CreateLoad(V);
  auto paced = PartsIntr::pac_pointer(builder, M, loaded);
  builder->CreateStore(paced, V);

  return true;
}

void PartsOptGlobalsPass::writeTypeIds(Module &M, std::list<PARTS::type_id_t> &type_ids, const char *sectionName)
{
  for (auto type_id : type_ids) {
    ConstantInt* type_id_Constant = ConstantInt::get(Type::getInt64Ty(M.getContext()), type_id);

    GlobalVariable *g = new GlobalVariable(M, Type::getInt64Ty(M.getContext()), true, GlobalValue::PrivateLinkage, type_id_Constant);
    g->setExternallyInitialized(false);
    g->setSection(sectionName);
  }
}
