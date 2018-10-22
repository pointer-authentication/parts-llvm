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

#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/PARTS/PartsTypeMetadata.h"

namespace llvm {

namespace PARTS {

class PartsIntr {

public:
  static Value *pac_pointer(Function &F, Instruction &I, Value *V, const std::string &name = "");
  static Value *pac_pointer(IRBuilder<> *builder, Module &M, Value *V, const std::string &name = "", PartsTypeMetadata_ptr = nullptr);

  static Value *load_aut_pointer(Function &F, Instruction &I, PartsTypeMetadata_ptr PTMD);
  static Value *store_aut_pointer(Function &F, Instruction &I, PartsTypeMetadata_ptr PTMD);
};

} // PARTS

} // llvm

#endif //LLVM_PARTSINTR_H
