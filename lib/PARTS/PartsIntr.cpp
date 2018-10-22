//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/PARTS/PartsIntr.h>

#include "llvm/PARTS/PartsIntr.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace llvm::PARTS;

Value *PartsIntr::pac_pointer(IRBuilder<> *builder, Module &M, Value *V, const std::string &name, PartsTypeMetadata_ptr PTMD) {
  // Get the intrinsic declaration based on our specific pointer type
  Type *arg_types[] = { V->getType() };
  if (PTMD == nullptr)
    PTMD = PartsTypeMetadata::get(V->getType());

  auto pacIntr = Intrinsic::getDeclaration(&M,
                                           (PTMD->isCodePointer() ? Intrinsic::pa_pacia : Intrinsic::pa_pacda),
                                           arg_types);

  auto typeIdConstant = PTMD->getTypeIdConstant(M.getContext());

  // Create the arguments for the intrinsic call (i.e., original pointer + modifier/type_id)
  Value *args[] = { V, typeIdConstant };
  return builder->CreateCall(pacIntr, args, name);
}

Value *PartsIntr::pac_pointer(Function &F, Instruction &I, Value *V, const std::string &name) {
  // insert the call
  IRBuilder<> Builder(&I);
  return pac_pointer(&Builder, *F.getParent(), V, name);
}

Value *PartsIntr::load_aut_pointer(Function &F, Instruction &I, PartsTypeMetadata_ptr partsMD) {
  assert(partsMD->isPointer());

  IRBuilder<> Builder(&I);

  //auto *mod = Builder.CreateLoad(partsMD->getTypeIdConstant(F.getContext()));
  auto *mod = partsMD->getTypeIdConstant(F.getContext());
  auto *ptr = Builder.Insert(I.clone());

  // Insert the unPAC/AUT intrinsic

  Type *arg_types[] = { I.getType() };
  //(errs
  I.dump();
  auto aut = partsMD->isCodePointer() ?
             Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_autia, arg_types) :
             Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_autda, arg_types);

  Value *args[] { ptr, mod };
  auto *V = Builder.CreateCall(aut, args, "unPACed_");

  // Replace uses of old instruction with new one
  I.replaceAllUsesWith(V);

  // Don't remove, optimizer should maybe get rid of this anyway?
  //I.removeFromParent();

  //return V;
  return nullptr;
}

Value *PartsIntr::store_aut_pointer(Function &F, Instruction &I, PartsTypeMetadata_ptr PTMD) {
  assert(PTMD->isPointer());

  IRBuilder<> Builder(&I);

  //auto *mod = Builder.CreateLoad(partsMD->getTypeIdConstant(F.getContext()));
  auto *mod = PTMD->getTypeIdConstant(F.getContext());
  auto *ptr = Builder.Insert(I.clone());

  // Insert the unPAC/AUT intrinsic

  Type *arg_types[] = { I.getType() };
  auto aut = PTMD->isCodePointer() ?
             Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_pacia, arg_types) :
             Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_pacda, arg_types);

  Value *args[] { ptr, mod };
  Builder.CreateCall(aut, args, "PACed_");

  // Replace uses of old instruction with new one
  //I.replaceAllUsesWith(V);

  // Don't remove, optimizer should maybe get rid of this anyway?
  //I.removeFromParent();

  //return V;
  return nullptr;
}


