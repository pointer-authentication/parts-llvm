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

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/Metadata.h"

namespace llvm {

namespace PARTS {

//typedef uint64_t type_id_t; // TODO I suggest to change to conform more LLVM convention?
Constant *getTypeIDConstantFrom(const Type &T, LLVMContext &C);

}

} // namespace llvm

#endif // LLVM_IR_PARTSTYPEMETADATA_H
