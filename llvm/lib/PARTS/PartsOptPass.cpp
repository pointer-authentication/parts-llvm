//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
//         Hans Liljestrand <hans@liljestrand.dev>
//         Gilang Mentari Hamidy <gilang.hamidy@gmail.com>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/PartsOptPass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Operator.h"
#include "llvm/PARTS/Parts.h"

using namespace llvm;
using namespace llvm::PARTS;

CallInst *PartsOptPass::createPartsIntrinsic(Function &F,
                                          Instruction &I,
                                          Value *calledValue,
                                          Intrinsic::ID intrinsicID) {
  const auto calledValueType = calledValue->getType();

  // Generate Builder for inserting PARTS intrinsic
  IRBuilder<> Builder(&I);
  // Get PARTS intrinsic declaration for correct input type
  auto autcall = Intrinsic::getDeclaration(F.getParent(), intrinsicID, { calledValueType });
  // Get type_id as Constant
  auto typeIdConstant = PARTS::getTypeIDConstantFrom(*calledValueType, F.getContext());
  // Insert PARTS intrinsics
  auto paced = Builder.CreateCall(autcall, { calledValue, typeIdConstant }, "");

  return paced;
}

CallInst *PartsOptPass::createPartsIntrinsicNoTypeID(Function &F,
                                          Instruction &I,
                                          Value *calledValue,
                                          Intrinsic::ID intrinsicID) {
  const auto calledValueType = calledValue->getType();

  // Generate Builder for inserting PARTS intrinsic
  IRBuilder<> Builder(&I);
  // Get PARTS intrinsic declaration for correct input type
  auto autcall = Intrinsic::getDeclaration(F.getParent(), intrinsicID, { calledValueType });
  // Insert PARTS intrinsics
  auto paced = Builder.CreateCall(autcall, { calledValue }, "");

  return paced;
}


bool PartsOptPass::isUnionMemberLoad(LoadInst *load) {
   auto defVal = load->getOperandUse(load->getPointerOperandIndex()).get();
   if (!isa<BitCastInst>(defVal) && !isa<BitCastOperator>(defVal))
      return false;

  Type *bcSrcType;
  if (isa<BitCastInst>(defVal))
    bcSrcType = dyn_cast<BitCastInst>(defVal)->getSrcTy();
  else
    bcSrcType = dyn_cast<BitCastOperator>(defVal)->getSrcTy();

  if (!isa<PointerType>(bcSrcType))
      return false;

  auto bcSrcPointerType = dyn_cast<PointerType>(bcSrcType);
  if (!bcSrcPointerType->getElementType()->isStructTy())
      return false;

  auto unionType = dyn_cast<StructType>(bcSrcPointerType->getElementType());
  if (!unionType->getName().startswith("union."))
      return false;

  return true;
}
