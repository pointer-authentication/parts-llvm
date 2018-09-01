//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTSINTR_H
#define LLVM_PARTSINTR_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"

namespace llvm {

namespace PARTS {

class PartsIntr {

public:
  static Value *pac_code_pointer(Function &F, Instruction &I, Value *V);
  static Value *pac_code_pointer(Function &F, Instruction &I, Value *V, const std::string &name);

  static Value *pac_pointer(Function &function, Instruction &instruction, Value *pValue, Function *pFunction,
                            const std::string &basic_string);
};

} // PARTS

} // llvm

#endif //LLVM_PARTSINTR_H
