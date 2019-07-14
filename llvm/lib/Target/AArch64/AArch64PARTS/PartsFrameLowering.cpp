//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PartsFrameLowering.h"
#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64RegisterInfo.h"
#include "PartsUtils.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/PARTS/Parts.h"

using namespace llvm;
using namespace llvm::PARTS;

static bool doInstrument(const MachineFunction &MF) {
  const Function &F = MF.getFunction();
  if (F.hasFnAttribute("no-parts") || !F.hasFnAttribute("parts-function_id")) return false;

  // Skip if we don't spill LR
  for (const auto &Info : MF.getFrameInfo().getCalleeSavedInfo())
    if (Info.getReg() != AArch64::LR)
      return true;

  return false;
}

void PartsFrameLowering::instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                  MachineBasicBlock &MBB) {
  if (!doInstrument(*MBB.getParent()))
    return;

  auto partsUtils = PartsUtils::get(TRI, TII);
  auto modReg = PARTS::getModifierReg();
  auto loc = MBB.getFirstTerminator();

  DebugLoc DL;
  if (loc != MBB.end())
    DL = loc->getDebugLoc();

  partsUtils->createBeCfiModifier(MBB, &*loc, modReg, DebugLoc());
  partsUtils->insertPAInstr(MBB, &*loc, AArch64::LR, modReg, TII->get(AArch64::AUTIB), DL);
}

void PartsFrameLowering::instrumentPrologue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                            MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                                            const DebugLoc &DL) {
  if (!doInstrument(*MBB.getParent()))
    return;

  auto partsUtils = PartsUtils::get(TRI, TII);
  auto modReg = PARTS::getModifierReg();

  partsUtils->createBeCfiModifier(MBB, &*MBBI, modReg, DebugLoc());
  partsUtils->insertPAInstr(MBB, &*MBBI, AArch64::LR, modReg, TII->get(AArch64::PACIB), DebugLoc());
}
