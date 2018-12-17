//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
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

Value *PartsIntr::aut_pointer(IRBuilder<> *builder, Module &M, Value *V, const std::string &name, PartsTypeMetadata_ptr PTMD) {
  // Get the intrinsic declaration based on our specific pointer type
  Type *arg_types[] = { V->getType() };
  if (PTMD == nullptr)
    PTMD = PartsTypeMetadata::get(V->getType());

  auto intr = Intrinsic::getDeclaration(&M,
                                        (PTMD->isCodePointer() ? Intrinsic::pa_autia : Intrinsic::pa_autda),
                                        arg_types);

  auto typeIdConstant = PTMD->getTypeIdConstant(M.getContext());

  // Create the arguments for the intrinsic call (i.e., original pointer + modifier/type_id)
  Value *args[] = { V, typeIdConstant };
  return builder->CreateCall(intr, args, name);
}

Value *PartsIntr::aut_pointer(Function &F, Instruction &I, Value *V, const std::string &name) {
  // insert the call
  IRBuilder<> Builder(&I);
  return aut_pointer(&Builder, *F.getParent(), V, name);
}
