//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTS_H
#define LLVM_PARTS_H

#include "llvm/IR/Constant.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"

namespace llvm {

namespace PARTS {

enum PartsBeCfiType{
  PartsBeCfiNone,
  PartsBeCfiFull,
  PartsBeCfiNgFull
};

enum PartsFeCfiType {
  PartsFeCfiNone,
  PartsFeCfiFull,
  PartsFeCfiFullNoType
};

bool useBeCfi();
bool useFeCfi();
bool useDpi();
bool isUnionTypePunningSupported(void);
bool useAny();
bool useDummy();
bool useRuntimeStats();

Constant *getTypeIDConstantFrom(const Type &T, LLVMContext &C);
PartsBeCfiType getBeCfiType();
PartsFeCfiType getFeCfiType();

Pass *createPartsOptCpiPass();
Pass *createPartsOptDataPointerArgsPass();
Pass *createPartsOptDpiPass();
Pass *createPartsOptGlobalsPass();
Pass *createPartsOptMainArgsPass();
Pass *createPartsOptRasPass();

} // PARTS

} // llvm

#endif //LLVM_PARTS_H
