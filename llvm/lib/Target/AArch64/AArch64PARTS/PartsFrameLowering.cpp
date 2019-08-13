//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Zaheer Ahmed Gauhar <zaheer.gauhar@aalto.fi>
//         Gilang Mentari Hamidy <gilang.hamidy@gmail.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PartsFrameLowering.h"
#include "AArch64PartsPassCommon.h"
#include "AArch64InstrInfo.h"
#include "AArch64MachineFunctionInfo.h"
#include "AArch64RegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/PARTS/Parts.h"
#include <sstream>

using namespace llvm;
using namespace llvm::PARTS;

static bool doInstrument(const MachineFunction &MF) {
  static const auto becfi = PARTS::getBeCfiType();

  // Just skip if we're not even using backward-edge CFI
  if (becfi == PartsBeCfiNone) return false;

  const Function &F = MF.getFunction();
  // Ignore function with the no-parts attribute
  if (F.hasFnAttribute("no-parts")) return false;
  // Ignore function without function-id (if we need it)
  if (becfi == PartsBeCfiFull && !F.hasFnAttribute("parts-function_id")) return false;

  // Skip if we don't spill LR
  for (const auto &Info : MF.getFrameInfo().getCalleeSavedInfo())
    if (Info.getReg() != AArch64::LR)
      return true;

  return false;
}

static void createBeCfiModifier(const TargetInstrInfo *TII, MachineBasicBlock &MBB, MachineInstr *MIi,
                                const unsigned modReg, const DebugLoc &DL) {
  auto &F = MBB.getParent()->getFunction();

  assert(F.hasFnAttribute("parts-function_id")  && "missing parts-function_id attribute");

  // auto type_id = PartsTypeMetadata::idFromType(F.getType());
  uint64_t type_id; // FIXME: uint64_t value passed as string
  std::istringstream iss(F.getFnAttribute("parts-function_id").getValueAsString());
  iss >> type_id;

  const auto t1 = ((type_id) % UINT16_MAX);
  const auto t2 = ((type_id << 16) % UINT16_MAX);
  const auto t3 = ((type_id << 32) % UINT16_MAX);

  if (MIi == nullptr) {
    BuildMI(&MBB, DL, TII->get(AArch64::ADDXri), modReg).addReg(AArch64::SP).addImm(0).addImm(0);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(16);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(32);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t3).addImm(48);
  } else {
    BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXri), modReg).addReg(AArch64::SP).addImm(0).addImm(0);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(16);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(32);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t3).addImm(48);
  }
}

static void createFastBeCfiModifier(const TargetInstrInfo *TII, MachineBasicBlock &MBB, MachineInstr *MIi,
                                const unsigned modReg, const DebugLoc &DL) {
  assert(PARTS::getBeCfiType() == PartsBeCfiNgFull);
  const auto &F = MBB.getParent()->getFunction();

  if (MIi == nullptr) {
    BuildMI(&MBB, DL, TII->get(AArch64::ADR), modReg)
        .addGlobalAddress(&F);
    BuildMI(&MBB, DL, TII->get(AArch64::BFMXri))
        .addDef(modReg)
        .addUse(modReg)
        .addReg(AArch64::FP)
        .addImm(32)
        .addImm(31);
  } else {
    BuildMI(MBB, MIi, DL, TII->get(AArch64::ADR), modReg)
        .addGlobalAddress(&F);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::BFMXri))
        .addDef(modReg)
        .addUse(modReg)
        .addReg(AArch64::FP)
        .addImm(32)
        .addImm(31);
  }
}

void PartsFrameLowering::instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                  MachineBasicBlock &MBB) {
  if (!doInstrument(*MBB.getParent()))
    return;

  const auto loc = MBB.getFirstTerminator();
  auto *MI = (loc != MBB.end() ? &*loc : nullptr);

  if (PARTS::getBeCfiType() == PartsBeCfiFull)
    createBeCfiModifier(TII, MBB, MI, modReg, DebugLoc());
  else
    createFastBeCfiModifier(TII, MBB, MI, modReg, DebugLoc());

  AArch64PartsPassCommon::insertPACInstr(MBB, MI, AArch64::LR, modReg, TII->get(AArch64::AUTIB));
}

void PartsFrameLowering::instrumentPrologue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                                            MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                                            const DebugLoc &DL) {
  if (!doInstrument(*MBB.getParent()))
    return;

  if (PARTS::getBeCfiType() == PartsBeCfiFull)
    createBeCfiModifier(TII, MBB, &*MBBI, modReg, DebugLoc());
  else
    createFastBeCfiModifier(TII, MBB, &*MBBI, modReg, DebugLoc());

  AArch64PartsPassCommon::insertPACInstr(MBB, &*MBBI, AArch64::LR, modReg, TII->get(AArch64::PACIB));
}
