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

#define Pauth_ModifierReg AArch64::X23

namespace llvm {

namespace PARTS {

bool useBeCfi();
bool useFeCfi();
bool useDpi();
bool isUnionTypePunningSupported(void);
bool useAny();
bool useDummy();
bool useRuntimeStats();

} // PARTS

} // llvm

#endif //LLVM_PARTS_H
