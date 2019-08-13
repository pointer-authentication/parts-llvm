//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Carlos Chinea <carlos.chinea.perez@huawei.com>
//         Zaheer Ahmed Gauhar <zaheer.gauhar@aalto.fi>
//         Gilang Mentari Hamidy <gilang.hamidy@gmail.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PartsUtils.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/IR/Constants.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include <iomanip>

using namespace llvm;
using namespace llvm::PARTS;

PartsUtils::PartsUtils(const TargetRegisterInfo *TRI, const TargetInstrInfo *TII) :
    TII(TII),
    TRI(TRI)
{}

bool PartsUtils::isLoadOrStore(const MachineInstr &MI) {
  const auto opCode = MI.getOpcode();
  return isLoad(opCode) || isStore(opCode);
}

bool PartsUtils::isLoad(const MachineInstr &MI) {
  return isLoad(MI.getOpcode());
}

bool PartsUtils::isStore(const MachineInstr &MI) {
  return isStore(MI.getOpcode());
}

bool PartsUtils::isStore(const unsigned opCode) {
  switch(opCode) {
    default:
      return false;
    case AArch64::STRWpost:
    case AArch64::STURQi:
    case AArch64::STURXi:
    case AArch64::STURDi:
    case AArch64::STURWi:
    case AArch64::STURSi:
    case AArch64::STURHi:
    case AArch64::STURHHi:
    case AArch64::STURBi:
    case AArch64::STURBBi:
    case AArch64::STPQi:
    case AArch64::STNPQi:
    case AArch64::STRQui:
    case AArch64::STPXi:
    case AArch64::STPDi:
    case AArch64::STNPXi:
    case AArch64::STNPDi:
    case AArch64::STRXui:
    case AArch64::STRDui:
    case AArch64::STPWi:
    case AArch64::STPSi:
    case AArch64::STNPWi:
    case AArch64::STNPSi:
    case AArch64::STRWui:
    case AArch64::STRSui:
    case AArch64::STRHui:
    case AArch64::STRHHui:
    case AArch64::STRBui:
    case AArch64::STRBBui:
      return true;
  }
}

bool PartsUtils::isLoad(const unsigned opCode) {
  switch(opCode) {
    default:
      return false;
    case AArch64::LDPXi:
    case AArch64::LDPDi:
    case AArch64::LDRWpost:
    case AArch64::LDURQi:
    case AArch64::LDURXi:
    case AArch64::LDURDi:
    case AArch64::LDURWi:
    case AArch64::LDURSi:
    case AArch64::LDURSWi:
    case AArch64::LDURHi:
    case AArch64::LDURHHi:
    case AArch64::LDURSHXi:
    case AArch64::LDURSHWi:
    case AArch64::LDURBi:
    case AArch64::LDURBBi:
    case AArch64::LDURSBXi:
    case AArch64::LDURSBWi:
    case AArch64::LDPQi:
    case AArch64::LDNPQi:
    case AArch64::LDRQui:
    case AArch64::LDNPXi:
    case AArch64::LDNPDi:
    case AArch64::LDRXui:
    case AArch64::LDRDui:
    case AArch64::LDPWi:
    case AArch64::LDPSi:
    case AArch64::LDNPWi:
    case AArch64::LDNPSi:
    case AArch64::LDRWui:
    case AArch64::LDRSui:
    case AArch64::LDRSWui:
    case AArch64::LDRHui:
    case AArch64::LDRHHui:
    case AArch64::LDRBui:
    case AArch64::LDRBBui:
      return true;
  }
}

void PartsUtils::moveTypeIdToReg(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned modReg,
                                 type_id_t type_id, const DebugLoc &DL) {
  moveTypeIdToReg(MBB, (MBB.instr_end() == MIi ? nullptr : &*MIi), modReg, type_id, DL);
}

void PartsUtils::moveTypeIdToReg(MachineBasicBlock &MBB, MachineInstr *MIi, unsigned modReg,
                                 type_id_t type_id, const DebugLoc &DL) {
  const auto t1 = type_id & UINT16_MAX;
  const auto t2 = (type_id >> 16) & UINT16_MAX;
  const auto t3 = (type_id >> 32) & UINT16_MAX;
  const auto t4 = (type_id >> 48) & UINT16_MAX;

  if (MIi == nullptr) {
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(0);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(16);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t3).addImm(32);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t4).addImm(48);
  } else {

    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(0);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(16);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t3).addImm(32);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t4).addImm(48);
  }
}

void PartsUtils::insertPAInstr(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                               unsigned modReg, const MCInstrDesc &MCID, const DebugLoc &DL) {
  insertPAInstr(MBB, (MBB.instr_end() == MIi ? nullptr : &*MIi), ptrReg, modReg, MCID, DL);
}

void PartsUtils::insertPAInstr(MachineBasicBlock &MBB, MachineInstr *MIi, unsigned ptrReg,
                               unsigned modReg, const MCInstrDesc &MCID, const DebugLoc &DL) {
  if (!PARTS::useDummy()) {
    if (MIi == nullptr) {
      BuildMI(&MBB, DL, MCID).addReg(ptrReg).addReg(modReg);
    } else {
      BuildMI(MBB, MIi, DL, MCID, ptrReg).addReg(modReg);
    }
  } else {
    addNops(MBB, MIi, ptrReg, modReg, DL);
  }
}

void PartsUtils::convertPartIntrinsic(MachineBasicBlock &MBB, MachineInstr &MI, unsigned instr) {
  const auto &DL = MI.getDebugLoc();
  const unsigned dst = MI.getOperand(0).getReg();
  const unsigned src = MI.getOperand(1).getReg();
  unsigned mod = MI.getOperand(2).getReg();

  // Save the mod register if it is marked as killable!
  if (MI.getOperand(2).isKill()) {
    unsigned oldMod = mod;
    mod = PARTS::getModifierReg();
    BuildMI(MBB, MI, DL, TII->get(AArch64::ADDXri), mod).addReg(oldMod).addImm(0).addImm(0);
  }
  // Move the pointer to destination register
  BuildMI(MBB, MI, DL, TII->get(AArch64::ADDXri), dst).addReg(src).addImm(0).addImm(0);

  insertPAInstr(MBB, &MI, dst, mod, TII->get(instr), DL);

  // And finally, remove the intrinsic
  MI.removeFromParent();
}

void PartsUtils::pacCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id, const DebugLoc &DL) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id, DL);
  insertPAInstr(MBB, MIi, ptrReg, modReg, TII->get(AArch64::PACIA), DL);
}

void PartsUtils::pacDataPointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id, const DebugLoc &DL) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id, DL);
  insertPAInstr(MBB, MIi, ptrReg, modReg, TII->get(AArch64::PACDA), DL);
}

void PartsUtils::autCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id, const DebugLoc &DL) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id, DL);
  insertPAInstr(MBB, MIi, ptrReg, modReg, TII->get(AArch64::AUTIA), DL);
}


void PartsUtils::autDataPointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id, const DebugLoc &DL) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id, DL);
  insertPAInstr(MBB, MIi, ptrReg, modReg, TII->get(AArch64::AUTDA), DL);
}

void PartsUtils::addNops(MachineBasicBlock &MBB, MachineInstr *MI, unsigned ptrReg, unsigned modReg, const DebugLoc &DL) {
  if (MI == nullptr) {
    BuildMI(&MBB, DL, TII->get(AArch64::EORXri)).addReg(ptrReg).addReg(ptrReg).addImm(17);
    BuildMI(&MBB, DL, TII->get(AArch64::EORXri)).addReg(ptrReg).addReg(ptrReg).addImm(37);
    BuildMI(&MBB, DL, TII->get(AArch64::EORXri)).addReg(ptrReg).addReg(ptrReg).addImm(97);
    BuildMI(&MBB, DL, TII->get(AArch64::EORXrs)).addReg(ptrReg).addReg(ptrReg).addReg(modReg).addImm(0);
  } else {
    BuildMI(MBB, MI, DL, TII->get(AArch64::EORXri), ptrReg).addReg(ptrReg).addImm(17);
    BuildMI(MBB, MI, DL, TII->get(AArch64::EORXri), ptrReg).addReg(ptrReg).addImm(37);
    BuildMI(MBB, MI, DL, TII->get(AArch64::EORXri), ptrReg).addReg(ptrReg).addImm(97);
    BuildMI(MBB, MI, DL, TII->get(AArch64::EORXrs), ptrReg).addReg(ptrReg).addReg(modReg).addImm(0);
  }
}
void PartsUtils::addEventCallFunction(MachineBasicBlock &MBB, MachineInstr &MI,
                                      const DebugLoc &DL, Function *func) {
  if (PARTS::useRuntimeStats()) {

    // FIXME: Ugly hack. Better to avoid the spill if LR is killed
    auto &MRI = MBB.getParent()->getRegInfo();
    MRI.clearKillFlags(AArch64::LR);

    BuildMI(MBB, MI, DL, TII->get(AArch64::STRXpre))
        .addReg(AArch64::SP, RegState::Define)
        .addReg(AArch64::LR)
        .addReg(AArch64::SP)
        .addImm(-16);
    BuildMI(MBB, MI, DL, TII->get(AArch64::BL))
        .addGlobalAddress(func);
    BuildMI(MBB, MI, DL, TII->get(AArch64::LDRXpost))
        .addReg(AArch64::SP, RegState::Define)
        .addReg(AArch64::LR, RegState::Define)
        .addReg(AArch64::SP)
        .addImm(16);
  }
}
