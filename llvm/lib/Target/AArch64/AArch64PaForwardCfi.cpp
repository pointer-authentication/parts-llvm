/*
 * AArch64PaForwardCfi.cpp
 * Copyright (C) 2018 Secure Systems Group, Aalto University, ssg.aalto.fi
 * Author: Hans Liljestrand <liljestrandh@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */


#include <iostream>
#include <llvm/IR/PartsTypeMetadata.h>
#include "PointerAuthentication.h"
#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "AArch64RegisterInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "aarch64-pa-forwardcfi"

using namespace llvm;
using namespace llvm::PA;

namespace {
 class PaForwardCfi : public MachineFunctionPass {
 public:
   static char ID;

   PaForwardCfi() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return "pauth-sllow"; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

   PartsTypeMetadata_ptr inferPauthTypeIdRegBackwards(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI,
                                              unsigned targetReg);
   PartsTypeMetadata_ptr inferPauthTypeIdStackBackwards(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI,
                                                unsigned targetReg, unsigned reg, int64_t imm);

   bool instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, pauth_type_id type_id);
   bool instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, pauth_type_id type_id);

 private:
   inline bool registerFitsPointer(unsigned reg);
   inline bool checkIfRegInstrumentable(unsigned reg);

   bool emitPAModAndInstr(MachineBasicBlock &MBB, MachineInstr &MI, unsigned PAOpcode, unsigned reg, pauth_type_id);

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
 };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PaForwardCfiPass() {
  return new PaForwardCfi();
}

char PaForwardCfi::ID = 0;

bool PaForwardCfi::doInitialization(Module &M) {
  return false;
}

bool PaForwardCfi::runOnMachineFunction(MachineFunction &MF) {
  if (!ENABLE_PAUTH_SLLOW) return false;

  DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
  DEBUG_PA_MIR(&MF, errs() << KBLU << "function " << MF.getName() << '\n' << KNRM);

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();

  auto &C = MF.getFunction().getContext();

  for (auto &MBB : MF) {
    DEBUG_PA_MIR(&MF, do { errs() << KBLU << "\tBlock "; errs().write_escaped(MBB.getName()); } while(0));

    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      DEBUG_PA_OPT(&MF, do { errs() << "\t\t"; MIi->dump(); } while(false));

      const auto MIOpcode = MIi->getOpcode();
      auto partsType = PartsTypeMetadata::retrieve(*MIi);

      if (MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL) {
        /* ----------------------------- BL/BLR ---------------------------------------- */
        DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tfound a BL/BLR (" << TII->getName(MIOpcode) << ")\n");

        if (partsType == nullptr) {
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to figure out type_id\n");
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tUNIMPLEMENTED!!!\n");

          partsType = PartsTypeMetadata::getUnknown();
        }

        if (!partsType->isKnown()) {
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\ttype_id is unknown!\n");
          errs() << MIi->getDebugLoc() <<  ": type_id is unknown!\n";
          continue;
        }

        if (partsType->isIgnored()) {
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tmarked as ignored, skipping\n" << KNRM);
          continue;
        }

        if (!partsType->isPointer()) {
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tnot a pointer, skipping\n" << KNRM);
          continue;
        }

        DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\t going to instrument indirect call (type_id=" << partsType->getTypeId() << ")\n");

        assert(MIOpcode != AArch64::BL && "Whoops, thought this was never, maybe, gonna happen. I guess?");

        // Create the PAC modifier
        BuildMI(MBB, *MIi, DebugLoc(), TII->get(AArch64::MOVZXi))
            .addReg(Pauth_ModifierReg)
            .addImm(partsType->getTypeId())
            .addImm(0);

        // Swap out the branch to a auth+branch variant
        auto BMI = BuildMI(MBB, *MIi, MIi->getDebugLoc(), TII->get(AArch64::BLRAA));
        BMI.add(MIi->getOperand(0));
        BMI.addReg(Pauth_ModifierReg);

        // Remove the old instruction!
        auto &MI = *MIi;
        MIi--;
        MI.removeFromParent();
        //MIi->removeFromParent();
      } else if (PA::isLoad(*MIi) || PA::isStore(*MIi)) {
        /* ----------------------------- LOAD/STORE ---------------------------------------- */
        DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tfound a load/store (" << TII->getName(MIOpcode) << ")\n" << KNRM);

        if (partsType == nullptr) {
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to figure out type_id\n");
          auto Op = MIi->getOperand(0);
          const auto targetReg = Op.getReg();

          if (!checkIfRegInstrumentable(targetReg)) {
            partsType = PartsTypeMetadata::getIgnored();
          } else {
            if (PA::isStore(*MIi)) {
              partsType = inferPauthTypeIdRegBackwards(MF, MBB, *MIi, targetReg);
            } else {
              // FIXME: this only supports loads of type load reg [reg, imm]
              if (MIi->getOperand(2).isImm()) {
                partsType = inferPauthTypeIdStackBackwards(MF, MBB, *MIi, targetReg, MIi->getOperand(1).getReg(), MIi->getOperand(2).getImm());
              } else {
                DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tOMG! unexpected operands, is this a pair store thingy?\n");
                partsType = PartsTypeMetadata::getUnknown();
              }
            }
          }
          addPauthMDNode(MF.getFunction().getContext(), *MIi, partsType->getTypeId());

          MIi->addOperand(MachineOperand::CreateMetadata(partsType->getMDNode(C)));
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tstoring type_id (" << partsType->getTypeId() << ") in current MI\n" << KNRM);
        }

        if (!partsType->isKnown()) {
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\ttype_id is unknown!\n");
          errs() << MIi->getDebugLoc() <<  ": type_id is unknown!\n";
          continue;
        }

        if (partsType->isIgnored()) {
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tmarked as ignored, skipping\n" << KNRM);
          continue;
        }

        if (!partsType->isPointer()) {
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tnot a pointer, skipping\n" << KNRM);
          continue;
        }

        DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tis a " << (partsType->isDataPointer() ? "data" : "code") <<
                                 " pointer, instrumenting (type_id=" << partsType->getTypeId() << ")\n");

        auto reg = MIi->getOperand(0).getReg();
        if (PA::isStore(*MIi)) {
          const auto instrOpcode = (partsType->isDataPointer() ? AArch64::PACDA : AArch64::PACIA);
          emitPAModAndInstr(MBB, *MIi, instrOpcode, reg, partsType->getTypeId());
        } else {
          auto tmp = MIi;
          tmp++;
          const auto instrOpcode = (partsType->isDataPointer() ? AArch64::AUTDA : AArch64::AUTIA);
          emitPAModAndInstr(MBB, *tmp, instrOpcode, reg, partsType->getTypeId());
        }
      }
    }
  }

  return true;
}

inline bool PaForwardCfi::registerFitsPointer(unsigned reg)
{
  const auto RC = TRI->getMinimalPhysRegClass(reg);
  if (64 <= TRI->getRegSizeInBits(*RC)) {
    return true;
  }
  DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tregister not suitable for pointers\n");
  return false;
}

inline bool PaForwardCfi::checkIfRegInstrumentable(unsigned reg)
{
  if (reg == AArch64::FP || reg == AArch64::LR) {
    DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tignoring FP and LR registers\n");
    return false;
  }
  return registerFitsPointer(reg);
}

PartsTypeMetadata_ptr PaForwardCfi::inferPauthTypeIdRegBackwards(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI,
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
        const pauth_type_id type_id = createPauthTypeId(FT->getParamType(param_i));
        // TODO: Embedd type_id into instruction
        DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tfound matching operand(" << param_i << "), using its type_id (=" << type_id << ")\n");
        return PartsTypeMetadata::get(type_id);
      }
    }
  }

  DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tfailed to infer type_id\n")
  return PartsTypeMetadata::getUnknown();
}

PartsTypeMetadata_ptr PaForwardCfi::inferPauthTypeIdStackBackwards(MachineFunction &MF, MachineBasicBlock &MBB,
                                                           MachineInstr &MI, unsigned targetReg,
                                                           unsigned reg, int64_t imm) {

  DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to look for [" << TRI->getName(reg) << ", #" << imm << "]\n");

  auto mbb = MBB.getReverseIterator();
  auto MIi = MI.getReverseIterator();
  MIi++;

  while(true) {
    // FIXME: This is potentially unsafe! iterator might not match execution order!

    while (MIi != mbb->instr_rend()) {
      if (PA::isStore(*MIi)) {
        if (MIi->getNumOperands() >= 3) {
          auto Op1 = MIi->getOperand(1);
          auto Op2 = MIi->getOperand(2);

          if (Op1.getReg() == reg && Op2.getImm() == imm) {
            // Found a store targeting the same location!
            const pauth_type_id type_id = getPauthTypeId(*MIi);
            DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tfound matching store, using it's type_id (" << type_id << ")\n");
            return PartsTypeMetadata::get(type_id);
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

bool PaForwardCfi::instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, pauth_type_id type_id)
{
  return emitPAModAndInstr(MBB, MI, AArch64::PACDA, pointerReg, type_id);
}

bool PaForwardCfi::instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, pauth_type_id type_id)
{
  auto MI_iter = MI.getIterator();
  MI_iter++;

  return emitPAModAndInstr(MBB, *MI_iter, AArch64::AUTDA, pointerReg, type_id);
}

bool PaForwardCfi::emitPAModAndInstr(MachineBasicBlock &MBB, MachineInstr &MI, unsigned PAOpcode, unsigned reg, pauth_type_id type_id)
{
  //type_id = 0; // FIXME: currently using zero PA modifier!!!
  BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::MOVZXi)).addReg(Pauth_ModifierReg).addImm(type_id).addImm(0);
  BuildMI(MBB, MI, DebugLoc(), TII->get(PAOpcode)).addReg(reg).addReg(Pauth_ModifierReg);
  return true;
}
