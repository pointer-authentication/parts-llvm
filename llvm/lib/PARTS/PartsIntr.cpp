//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Gilang Mentari Hamidy <gilang.hamidy@gmail.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/PartsIntr.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/PARTS/Parts.h"

using namespace llvm;
using namespace llvm::PARTS;


Value *PartsIntr::pac_pointer(IRBuilder<> *builder, Module &M, Value *V,
                              const std::string &name) {
  // Get the intrinsic declaration based on our specific pointer type
  Type *arg_types[] = {V->getType()};

  auto type = V->getType();
  assert(type->isPointerTy() && "Value is not a pointer type");

  bool isFunctionPtr =
      type->isPointerTy() && type->getPointerElementType()->isFunctionTy();

  auto pacIntr = Intrinsic::getDeclaration(
      &M, (!isFunctionPtr ? Intrinsic::pa_pacda : Intrinsic::pa_pacia),
      arg_types);

  auto typeIdConstant = PARTS::getTypeIDConstantFrom(*type, M.getContext());

  // Create the arguments for the intrinsic call (i.e., original pointer +
  // modifier/type_id)
  Value *args[] = {V, typeIdConstant};
  return builder->CreateCall(pacIntr, args, name);
}

Value *PartsIntr::pac_pointer(Function &F, Instruction &I, Value *V,
                              const std::string &name) {
  // insert the call
  IRBuilder<> Builder(&I);
  return pac_pointer(&Builder, *F.getParent(), V, name);
}

Value *PartsIntr::aut_pointer(IRBuilder<> *builder, Module &M, Value *V,
                              const std::string &name) {
  // Get the intrinsic declaration based on our specific pointer type
  Type *arg_types[] = {V->getType()};
  auto type = V->getType();
  assert(type->isPointerTy() && "Value is not a pointer type");

  bool isFunctionPtr =
      type->isPointerTy() && type->getPointerElementType()->isFunctionTy();

  auto pacIntr = Intrinsic::getDeclaration(
      &M, (!isFunctionPtr ? Intrinsic::pa_pacda : Intrinsic::pa_pacia),
      arg_types);

  auto typeIdConstant = PARTS::getTypeIDConstantFrom(*type, M.getContext());

  // Create the arguments for the intrinsic call (i.e., original pointer +
  // modifier/type_id)
  Value *args[] = {V, typeIdConstant};
  return builder->CreateCall(pacIntr, args, name);
}

Value *PartsIntr::aut_pointer(Function &F, Instruction &I, Value *V,
                              const std::string &name) {
  // insert the call
  IRBuilder<> Builder(&I);
  return aut_pointer(&Builder, *F.getParent(), V, name);
}
