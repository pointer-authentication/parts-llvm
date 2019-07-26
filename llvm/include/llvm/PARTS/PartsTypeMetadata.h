//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_PARTSTYPEMETADATA_H
#define LLVM_IR_PARTSTYPEMETADATA_H

#include "llvm/IR/Constant.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"

namespace llvm {
namespace PARTS {

Constant *getTypeIDConstantFrom(const Type &T, LLVMContext &C);

}
}

#endif
