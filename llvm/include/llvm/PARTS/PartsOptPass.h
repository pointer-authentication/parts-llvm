//===----------------------------------------------------------------------===//
//
// Authors: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __PARTSOPTPASS_H__
#define __PARTSOPTPASS_H__

#include "llvm/PARTS/PartsTypeMetadata.h"

using namespace llvm;
using namespace llvm::PARTS;

namespace llvm {
namespace PARTS {
namespace PartsOptPass {
  static inline CallInst *createPartsIntrinsic(Function &F, Instruction &I, Value *calledValue, Intrinsic::ID intrinsicID);
  static inline CallInst *createPartsIntrinsicNoTypeID(Function &F, Instruction &I, Value *calledValue, Intrinsic::ID intrinsicID);
  static inline bool isUnionMemberLoad(LoadInst *load);
  static inline bool isCodePointer(const Type *const type);
  static inline bool isDataPointer(const Type *const type);
}; // PartOptPass
}; // PARTS
}; // llvm

static inline bool PartsOptPass::isCodePointer(const Type *const type) {
  return type->isPointerTy() && type->getPointerElementType()->isFunctionTy();
}

static inline bool PartsOptPass::isDataPointer(const Type *const type) {
  return type->isPointerTy() && !type->getPointerElementType()->isFunctionTy();
}

static inline CallInst *PartsOptPass::createPartsIntrinsic(Function &F,
                                          Instruction &I,
                                          Value *calledValue,
                                          Intrinsic::ID intrinsicID) {
  const auto calledValueType = calledValue->getType();

  // Generate Builder for inserting PARTS intrinsic
  IRBuilder<> Builder(&I);
  // Get PARTS intrinsic declaration for correct input type
  auto autcall = Intrinsic::getDeclaration(F.getParent(), intrinsicID, { calledValueType });
  // Get type_id as Constant
  auto typeIdConstant = PartsTypeMetadata::idConstantFromType(F.getContext(), calledValueType);
  // Insert PARTS intrinsics
  auto paced = Builder.CreateCall(autcall, { calledValue, typeIdConstant }, "");

  return paced;
}

static inline CallInst *PartsOptPass::createPartsIntrinsicNoTypeID(Function &F,
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

static inline bool PartsOptPass::isUnionMemberLoad(LoadInst *load) {
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
#endif // __PARTSOPTPASS_H_n_
