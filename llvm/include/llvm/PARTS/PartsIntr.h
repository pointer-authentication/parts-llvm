//
// Created by Hans Liljestrand on 24/08/18.
// Copyright (c) 2018 Hans Liljestrand. All rights reserved.
//

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
