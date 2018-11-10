//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/Constants.h>
#include "PartsUtils.h"

#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsLog.h"

using namespace llvm;
using namespace llvm::PARTS;

PartsUtils::PartsUtils(const TargetRegisterInfo *TRI, const TargetInstrInfo *TII) :
    log(PartsLog::getLogger("PartsUtils")),
    TII(TII),
    TRI(TRI)
{
  DEBUG_PA(log->enable());
};

PartsTypeMetadata_ptr PartsUtils::inferPauthTypeIdStackBackwards(MachineFunction &MF,
                                                                 MachineBasicBlock &MBB,
                                                                 MachineInstr &MI, unsigned targetReg,
                                                                 unsigned reg, int64_t imm) {
  const auto fName = MF.getName();
  DEBUG_PA(log->info(fName) << "trying to look for [" << TRI->getName(reg) << ", #" << imm << "]\n");

  auto mbb = MBB.getReverseIterator();
  auto MIi = MI.getReverseIterator();
  MIi++;

  while(true) {
    // FIXME: This is potentially unsafe! iterator might not match execution order!

    while (MIi != mbb->instr_rend()) {
      if (isStore(*MIi)) {
        if (MIi->getNumOperands() >= 3) {
          auto Op1 = MIi->getOperand(1);
          auto Op2 = MIi->getOperand(2);

          if (Op1.isReg() && Op2.isImm() && Op1.getReg() == reg && Op2.getImm() == imm) {
            // Found a store targeting the same location!
            const auto PTMD = PartsTypeMetadata::retrieve(*MIi);
            log->inc("PartsUtils.BackwardsLookupOk", true, fName) << "found matching store " << PTMD->toString() << "\n";
            return PTMD;
          }
        }
      }

      MIi++;
    }

    mbb++;

    if (mbb == MF.rend())
      break;

    // Do this update there, so we can start the first MBB iteration at MI
    MIi = mbb->instr_rbegin();
  }

  log->inc("PartsUtils.BackwardsLookupFail", false, fName) << "failed to infer type_id\n";
  return PartsTypeMetadata::getUnknown();
}


PartsTypeMetadata_ptr PartsUtils::inferPauthTypeIdRegBackwards(MachineFunction &MF,
                                                               MachineBasicBlock &MBB,
                                                               MachineInstr &MI,
                                                               unsigned targetReg) {
  const auto fName = MF.getName();
  auto iter = MI.getIterator();

  DEBUG_PA(log->info(fName) << "trying to look for " << TRI->getName(targetReg) << " load\n");

  // Look through current MBB
  log->error(fName) << __FUNCTION__ << ": backward MBB lookup UNIMPLEMENTED!!!!\n";
  while (false && iter != MBB.begin()) {
    iter--;

    for (unsigned i = 0; i < iter->getNumOperands(); i++) {
      const MachineOperand &MO = iter->getOperand(i);

      if (MO.isReg()) {
        if (MO.getReg() == targetReg) {
          DEBUG_PA(log->info(fName) << "      used in " << TII->getName(iter->getOpcode()) << "\n");
          // TODO: unimplemented!
          //llvm_unreachable_internal("unimplemented");
        }
      }
    }
  }

  // Check if this is the entry block, and if so, look at function arguments
  if (MF.begin() == MBB.getIterator()) {
    DEBUG_PA(log->info(fName) << "      trying to look at function args\n");
    auto *FT = MF.getFunction().getFunctionType();
    const auto numParams = FT->getNumParams();
    if (numParams != 0) {
      unsigned param_i = INT_MAX;

      switch(targetReg) {
        case AArch64::X0: param_i = 0; break;
        case AArch64::X1: param_i = 1; break;
        case AArch64::X2: param_i = 2; break;
        case AArch64::X3: param_i = 3; break;
        case AArch64::X4: param_i = 4; break;
        case AArch64::X5: param_i = 5; break;
        case AArch64::X6: param_i = 6; break;
        default: break;
      }

      if (param_i < numParams) {
        const auto PTMD = PartsTypeMetadata::get(FT->getParamType(param_i));
        // TODO: Embedd type_id into instruction
        log->inc("PartsUtils.ForwardLookupFunc", true, fName) << "      found matching operand(" << param_i <<
                                                              "), using " << PTMD->toString() << "\n";
        return PTMD;
      }
    }
  }

  log->inc("PartsUtils.ForwardsLookupFail", false, fName) << "      failed to infer type_id\n";
  return PartsTypeMetadata::getUnknown();
}

void PartsUtils::attach(LLVMContext &C, PartsTypeMetadata_ptr PTMD, MachineInstr *MI) {
  MI->addOperand(MachineOperand::CreateMetadata(PTMD->getMDNode(C)));
}

void PartsUtils::attach(LLVMContext &C, PartsTypeMetadata_ptr PTMD, MachineInstrBuilder *MIB) {
  MIB->addMetadata(PTMD->getMDNode(C));
}

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

#include <iomanip>

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

void PartsUtils::createBeCfiModifier(MachineBasicBlock &MBB, MachineInstr *MIi, unsigned modReg, const DebugLoc &DL) {
  auto &F = MBB.getParent()->getFunction();
  auto type_id = PartsTypeMetadata::idFromType(F.getType());

  const auto f1 = 1234;
  const auto t1 = ((type_id) % UINT16_MAX);
  const auto t2 = ((type_id << 16) % UINT16_MAX);

  if (MIi == nullptr) {
    BuildMI(&MBB, DL, TII->get(AArch64::ADDXri), modReg).addReg(AArch64::SP).addImm(0).addImm(0);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(f1).addImm(16);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(32);
    BuildMI(&MBB, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(48);
  } else {
    BuildMI(MBB, MIi, DL, TII->get(AArch64::ADDXri), modReg).addReg(AArch64::SP).addImm(0).addImm(0);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(f1).addImm(16);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t1).addImm(32);
    BuildMI(MBB, MIi, DL, TII->get(AArch64::MOVKXi), modReg).addReg(modReg).addImm(t2).addImm(48);
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
    auto BMI = BuildMI(MBB, MI, DL, TII->get(AArch64::BL));
    BMI.addGlobalAddress(func);
  }
}

