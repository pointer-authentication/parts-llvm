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

#include "llvm/Pass.h"

namespace llvm {

namespace PARTS {

enum PartsBeCfiType{
  PartsBeCfiNone,
  PartsBeCfiFull
};

bool useBeCfi();
bool useFeCfi();
bool useDpi();
bool isUnionTypePunningSupported(void);
bool useAny();
bool useDummy();
bool useRuntimeStats();

Pass *createPartsOptCpiPass();
Pass *createPartsOptDataPointerArgsPass();
Pass *createPartsOptDpiPass();
Pass *createPartsOptGlobalsPass();
Pass *createPartsOptMainArgsPass();
Pass *createPartsOptRasPass();

} // PARTS

} // llvm

#endif //LLVM_PARTS_H
