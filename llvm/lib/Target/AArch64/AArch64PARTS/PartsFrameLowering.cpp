//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PartsFrameLowering.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

PartsFrameLowering_ptr PartsFrameLowering::get() {
  return std::make_shared<PartsFrameLowering>();
}

void PartsFrameLowering::instrumentEpilogue(const TargetInstrInfo *TII,
                                  MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                                  const DebugLoc &DL, const bool IsTailCallReturn) {
  if (!IsTailCallReturn) {
    assert(MBBI != MBB.end());
    BuildMI(MBB, MBBI, DebugLoc(), TII->get(AArch64::RETAA));
    MBB.erase(MBBI);
  } else {
    BuildMI(MBB, MBBI, DebugLoc(), TII->get(AArch64::AUTIASP));
  }
}

void PartsFrameLowering::instrumentPrologue(const TargetInstrInfo *TII,
                        MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                        const DebugLoc &DL)
{
  BuildMI(MBB, MBBI, DebugLoc(), TII->get(AArch64::PACIASP));
}
