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
#include "AArch64RegisterInfo.h"
#include "AArch64InstrInfo.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

using namespace llvm;

PartsFrameLowering_ptr PartsFrameLowering::get() {
  return std::make_shared<PartsFrameLowering>();
}

void PartsFrameLowering::instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                  MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                                  const DebugLoc &DL, const bool IsTailCallReturn) {
  auto partsUtils = PartsUtils::get(TRI, TII);

  if (!IsTailCallReturn) {
    assert(MBBI != MBB.end());
    partsUtils->moveTypeIdToReg(MBB, &*MBBI, Pauth_ModifierReg, 0, DebugLoc());
    BuildMI(MBB, MBBI, DL, TII->get(AArch64::ADDXri), Pauth_ModifierReg).addReg(AArch64::SP).addImm(0).addImm(0);
    partsUtils->insertPAInstr(MBB, &*MBBI, AArch64::LR, Pauth_ModifierReg, TII->get(AArch64::AUTIB), DebugLoc());
  } else {
    partsUtils->moveTypeIdToReg(MBB, &*MBBI, Pauth_ModifierReg, 0, DebugLoc());
    BuildMI(MBB, MBBI, DL, TII->get(AArch64::ADDXri), Pauth_ModifierReg).addReg(AArch64::SP).addImm(0).addImm(0);
    partsUtils->insertPAInstr(MBB, &*MBBI, AArch64::LR, Pauth_ModifierReg, TII->get(AArch64::AUTIB), DebugLoc());
  }
}

void PartsFrameLowering::instrumentPrologue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                            MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                                            const DebugLoc &DL) {
  auto partsUtils = PartsUtils::get(TRI, TII);

  partsUtils->moveTypeIdToReg(MBB, &*MBBI, Pauth_ModifierReg, 0, DebugLoc());
  BuildMI(MBB, MBBI, DL, TII->get(AArch64::ADDXri), Pauth_ModifierReg).addReg(AArch64::SP).addImm(0).addImm(0);
  partsUtils->insertPAInstr(MBB, &*MBBI, AArch64::LR, Pauth_ModifierReg, TII->get(AArch64::PACIB), DebugLoc());
}
