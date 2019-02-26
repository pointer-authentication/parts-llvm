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

class PartsOptPass {
protected:
  inline Value *createPartsIntrinsic(Function &F, Instruction &I, Value *calledValue, Intrinsic::ID intrinsicID);
  inline bool isCodePointer(const Type *const type);
  inline bool isDataPointer(const Type *const type);
};

};
};

inline bool PartsOptPass::isCodePointer(const Type *const type) {
  return type->isPointerTy() && type->getPointerElementType()->isFunctionTy();
}

inline bool PartsOptPass::isDataPointer(const Type *const type) {
  return type->isPointerTy() && !type->getPointerElementType()->isFunctionTy();
}

inline Value *PartsOptPass::createPartsIntrinsic(Function &F,
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

#endif // __PARTSOPTPASS_H__
