//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTS_H
#define LLVM_PARTS_H

#define Pauth_ModifierReg AArch64::X23

//#define USE_DUMMY_INSTRUCTIONS

namespace llvm {

namespace PARTS {

bool useBeCfi();
bool useFeCfi();
bool useDpi();
bool useAny();

} // PARTS

} // llvm

#endif //LLVM_PARTS_H
