//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Zaheer Ahmed Gauhar <zaheer.gauhar@aalto.fi>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTSFRAMELOWERING_H
#define LLVM_PARTSFRAMELOWERING_H

#include "AArch64InstrInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"

namespace llvm {
namespace PartsFrameLowering {

constexpr const unsigned modReg = AArch64::X16;

void instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI, MachineBasicBlock &MBB);
void instrumentPrologue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                        MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                        const DebugLoc &DL);

}
}

#endif //LLVM_PARTSFRAMELOWERING_H
