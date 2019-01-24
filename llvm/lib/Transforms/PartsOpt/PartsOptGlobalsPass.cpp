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
#include "llvm/PARTS/PartsLog.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include <llvm/Transforms/Utils/ModuleUtils.h>

using namespace llvm;

#define DEBUG_TYPE "PartsOptGlobalsPass"
#define TAG KYEL DEBUG_TYPE ": "

//#undef DEBUG_PA
//#define DEBUG_PA(x) x

namespace {

struct PartsOptGlobalsPass: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  PartsLog_ptr log;

  std::list<PARTS::type_id_t> data_type_ids = std::list<PARTS::type_id_t>(0);
  std::list<PARTS::type_id_t> code_type_ids = std::list<PARTS::type_id_t>(0);
  unsigned marked_data_pointers = 0;
  unsigned marked_code_pointers = 0;
  unsigned fixed_dp = 0;
  unsigned fixed_cp = 0;
  bool need_fix_globals_call = false;

  IRBuilder<> *builder;

  Function *funcFixGlobals = nullptr;

  PartsOptGlobalsPass() :
      FunctionPass(ID),
      log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool doInitialization(Module &M) override;
  bool runOnFunction(Function &M) override;

  bool handle(Module &M, Value *V, Type *Ty);
  bool handle(Module &M, Value *V, StructType *Ty);
  bool handle(Module &M, Value *V, ArrayType *Ty);
  bool handle(Module &M, Value *V, PointerType *Ty);

private:
  void writeTypeIds(Module &M, std::list<PARTS::type_id_t> &type_ids, const char *sectionName);

};

} // anonymous namespace

char PartsOptGlobalsPass::ID = 0;
static RegisterPass<PartsOptGlobalsPass> X("parts-opt-globals", "PARTS globals fix, needed for CPI and DPI");

bool PartsOptGlobalsPass::doInitialization(Module &M) {
  if ( !(PARTS::useFeCfi() || PARTS::useDpi())) // We don't need to do anything unless we use PI
    return false;

  auto &C = M.getContext();

  auto result = Type::getVoidTy(C);
  FunctionType* signature = FunctionType::get(result, false);
  funcFixGlobals = Function::Create(signature, Function::ExternalLinkage, "__pauth_pac_globals", &M);
  funcFixGlobals->addFnAttr("no-parts", "true");
 // funcFixGlobals->addFnAttr("noinline", "true");

  auto BB = BasicBlock::Create(M.getContext(), "entry", funcFixGlobals);
  IRBuilder<> localBuilder(BB);
  builder = &localBuilder;

  for (auto GI = M.global_begin(); GI != M.global_end(); GI++) {

    if (GI->getNumOperands() == 0) {
      DEBUG_PA(log->info() << "skipping empty\n");
    } else {
      DEBUG_PA(log->info() << "inspecting " << GI << "\n");
      const auto O = GI->getOperand(0);
      handle(M, &*GI, O->getType());
    }

  }
  builder->CreateRetVoid();
  builder = nullptr;

  appendToGlobalCtors(M, funcFixGlobals, 0);

  if (PARTS::useFeCfi()) {
    log->inc(DEBUG_TYPE ".CodePointersFixed", fixed_cp) << "\"fixed\" " << fixed_cp << " code pointers for PACing\n";
    log->inc(DEBUG_TYPE ".CodePointersMarked", marked_code_pointers) << "annotating " << marked_code_pointers << " code pointers for PACing\n";
    writeTypeIds(M, code_type_ids, ".code_type_id");
  }

  if (PARTS::useDpi()) {
    log->inc(DEBUG_TYPE ".DataPointersFixed", fixed_dp) << "\"fixed\" " << fixed_dp << " data pointers for PACing\n";
    log->inc(DEBUG_TYPE ".DataPointersMarked", marked_data_pointers) << "annotating " << marked_data_pointers << " data pointers for PACing\n";
    writeTypeIds(M, data_type_ids, ".data_type_id");
  }

  need_fix_globals_call = (marked_code_pointers+marked_data_pointers+fixed_cp+fixed_dp) > 0;
  return need_fix_globals_call;
}

bool PartsOptGlobalsPass::runOnFunction(Function &F) {
  return false;
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
    DEBUG_PA(log->debug() << "Getting GEP to " << base << ", " << i << "\n");
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

  auto type_id = PTMD->getTypeId();

  if (PTMD->isCodePointer()) {
    if (PARTS::useFeCfi()) {
      marked_code_pointers++;
      log->debug() << "mark as code pointer type_id=" << type_id << "\n";
    } else {
      PTMD->setIgnored(true);
    }
  } else {
    assert(PTMD->isDataPointer());
    if (PARTS::useDpi()) {
      marked_data_pointers++;
      log->green() << "mark as data pointer type_id=" << type_id << "\n";
    } else {
      PTMD->setIgnored(true);
    }
  }

  if (PTMD->isIgnored()) {
    DEBUG_PA(log->info() << "skipping ignored\n");
    return false;
  }

  DEBUG_PA(log->debug() << "inserting new PAC call to global fixer function\n");

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
