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
  auto modReg = PARTS::getModifierReg();

  if (!IsTailCallReturn) {
    assert(MBBI != MBB.end());
    partsUtils->createBeCfiModifier(MBB, &*MBBI, modReg, DebugLoc());
    partsUtils->insertPAInstr(MBB, &*MBBI, AArch64::LR, modReg, TII->get(AArch64::AUTIB), DebugLoc());
  } else {
    partsUtils->createBeCfiModifier(MBB, &*MBBI, modReg, DebugLoc());
    partsUtils->insertPAInstr(MBB, &*MBBI, AArch64::LR, modReg, TII->get(AArch64::AUTIB), DebugLoc());
  }
}

void PartsFrameLowering::instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                  MachineBasicBlock &MBB, const DebugLoc &DL, const bool IsTailCallReturn) {
  auto partsUtils = PartsUtils::get(TRI, TII);
  auto modReg = PARTS::getModifierReg();

  if (!IsTailCallReturn) {
    partsUtils->createBeCfiModifier(MBB, nullptr, modReg, DebugLoc());
    partsUtils->insertPAInstr(MBB, nullptr, AArch64::LR, modReg, TII->get(AArch64::AUTIB), DebugLoc());
  } else {
    partsUtils->createBeCfiModifier(MBB, nullptr, modReg, DebugLoc());
    partsUtils->insertPAInstr(MBB, nullptr, AArch64::LR, modReg, TII->get(AArch64::AUTIB), DebugLoc());
  }
}

void PartsFrameLowering::instrumentPrologue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                            MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                                            const DebugLoc &DL) {
  auto partsUtils = PartsUtils::get(TRI, TII);
  auto modReg = PARTS::getModifierReg();

  partsUtils->createBeCfiModifier(MBB, &*MBBI, modReg, DebugLoc());
  partsUtils->insertPAInstr(MBB, &*MBBI, AArch64::LR, modReg, TII->get(AArch64::PACIB), DebugLoc());
}
