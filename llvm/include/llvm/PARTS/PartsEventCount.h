//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@aalto.fi>
// Copyright (c) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_PARTSEVENTCOUNT_H
#define LLVM_PARTSEVENTCOUNT_H

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

namespace llvm {

namespace PARTS {

class PartsEventCount {

public:
  static Function *getFuncCodePointerBranch(Module &M) { return getCounterFunc(M, "__parts_count_code_ptr_branch"); };
  static Function *getFuncCodePointerCreate(Module &M) { return getCounterFunc(M, "__parts_count_code_ptr_create"); };

private:
  static Function *getCounterFunc(Module &M, const std::string &fName);
};

}

}

#endif //LLVM_PARTSEVENTCOUNT_H
