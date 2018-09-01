//
// Created by ishkamiel on 30/08/18.
//

#include "PartsUtils.h"

#include "llvm/PARTS/PartsLog.h"

using namespace llvm;
using namespace llvm::PARTS;

PartsUtils::PartsUtils(const AArch64RegisterInfo *TRI, const AArch64InstrInfo *TII)
    : TII(TII), TRI(TRI) {};

PartsTypeMetadata_ptr PartsUtils::inferPauthTypeIdStackBackwards(MachineFunction &MF,
                                                                 MachineBasicBlock &MBB,
                                                                 MachineInstr &MI, unsigned targetReg,
                                                                 unsigned reg, int64_t imm) {

  DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to look for [" << TRI->getName(reg) << ", #" << imm << "]\n");

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

          if (Op1.getReg() == reg && Op2.getImm() == imm) {
            // Found a store targeting the same location!
            const auto PTMD = PartsTypeMetadata::retrieve(*MIi);
            DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tfound matching store, using it's type_id (" << PTMD->getTypeId() << ")\n");
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

  DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tfailed to infer type_id\n")
  return PartsTypeMetadata::getUnknown();
}


PartsTypeMetadata_ptr PartsUtils::inferPauthTypeIdRegBackwards(MachineFunction &MF,
                                                               MachineBasicBlock &MBB,
                                                               MachineInstr &MI,
                                                               unsigned targetReg)
{
  auto iter = MI.getIterator();

  DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to look for " << TRI->getName(targetReg) << " load\n");

  // Look through current MBB
  while (iter != MBB.begin()) {
    iter--;

    for (unsigned i = 0; i < iter->getNumOperands(); i++) {
      const MachineOperand &MO = iter->getOperand(i);

      if (MO.isReg()) {
        if (MO.getReg() == targetReg) {
          DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tused in " << TII->getName(iter->getOpcode()) << "\n" << KNRM);
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tUNIMPLEMENTED!!!!\n");
          // TODO: unimplemetned!
          //llvm_unreachable_internal("unimplemented");
        }
      }
    }
  }

  // Check if this is the entry block, and if so, look at function arguments
  if (MF.begin() == MBB.getIterator()) {
    DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to look at function args\n");
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
        DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tfound matching operand(" << param_i << "), using its type_id (=" << PTMD->getTypeId() << ")\n");
        return PTMD;
      }
    }
  }

  DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tfailed to infer type_id\n")
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
                                 type_id_t type_id) {
  // FIXME: This doesn't seem to work with larger than 8(?) bit values!?! (value truncated in PartsTypeMetadata)
  //BuildMI(MBB, MIi, MIi->getDebugLoc(), TII->get(AArch64::MOVZXi), modReg).addImm((uint32_t)type_id) .addImm(0);
  //BuildMI(MBB, MIi, MIi->getDebugLoc(), TII->get(AArch64::MOVZXi), modReg).addImm((uint16_t)type_id) .addImm(0);
  BuildMI(MBB, MIi, MIi->getDebugLoc(), TII->get(AArch64::MOVZXi), modReg).addImm(type_id).addImm(0);
}

void PartsUtils::pacCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned dstReg,
                                unsigned srcReg, unsigned modReg, type_id_t type_id) {
  // Move src to dst
  BuildMI(MBB, MIi, MIi->getDebugLoc(), TII->get(AArch64::ADDXri), dstReg).addReg(srcReg).addImm(0).addImm(0);
  // Then PAC only the dst
  pacCodePointer(MBB, MIi, dstReg, modReg, type_id);
}

void PartsUtils::pacCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id);
  BuildMI(MBB, MIi, MIi->getDebugLoc(), TII->get(AArch64::PACIA), ptrReg).addReg(modReg);
}

void PartsUtils::pacDataPointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id);
  BuildMI(MBB, MIi, DebugLoc(), TII->get(AArch64::PACDA)).addReg(ptrReg).addReg(modReg);
}

void PartsUtils::autCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id);
  BuildMI(MBB, MIi, DebugLoc(), TII->get(AArch64::AUTIA)).addReg(ptrReg).addReg(modReg);
}


void PartsUtils::autDataPointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                                unsigned modReg, type_id_t type_id) {
  moveTypeIdToReg(MBB, MIi, modReg, type_id);
  BuildMI(MBB, MIi, DebugLoc(), TII->get(AArch64::AUTDA)).addReg(ptrReg).addReg(modReg);
}



