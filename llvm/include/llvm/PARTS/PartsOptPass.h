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

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/PARTS/PartsTypeMetadata.h"

namespace llvm {
namespace PARTS {
namespace PartsOptPass {

  CallInst *createPartsIntrinsic(Function &F, Instruction &I, Value *calledValue, Intrinsic::ID intrinsicID);
  CallInst *createPartsIntrinsicNoTypeID(Function &F, Instruction &I, Value *calledValue, Intrinsic::ID intrinsicID);
  bool isUnionMemberLoad(LoadInst *load);
  static inline bool isCodePointer(const Type *const type);
  static inline bool isDataPointer(const Type *const type);

static inline bool isCodePointer(const Type *const type) {
  return type->isPointerTy() && type->getPointerElementType()->isFunctionTy();
}

static inline bool isDataPointer(const Type *const type) {
  return type->isPointerTy() && !type->getPointerElementType()->isFunctionTy();
}


} // PartOptPass
} // PARTS
} // llvm


#endif // __PARTSOPTPASS_H_n_
