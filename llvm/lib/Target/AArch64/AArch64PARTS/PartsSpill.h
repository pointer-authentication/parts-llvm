//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea <carlos.chinea.perez@huawei.com>
// Copyright (C) 2019 Huawei Finland Oy
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTSSPILL_H
#define LLVM_PARTSSPILL_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"

namespace llvm {

#define PARTS_STACKID 42

static inline bool isUnprotectedDataPtr(MachineInstr &MI, unsigned Reg) {
  unsigned Opc = MI.getOpcode();

  switch (Opc) {
  case AArch64::PARTS_DATA_PTR:
    return true;
  case AArch64::PARTS_AUTDA: {
    auto DefReg = MI.getOperand(0).getReg();
    if (DefReg == Reg)
      return true;
    }
    break;
  default:
      break;
  }

 return false;
}

} // namespace llvm

#endif // LLVM_PARTSSPILL_H
