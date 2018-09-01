//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <iostream>
// LLVM includes
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
// PARTS includes
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/PARTS/PartsLog.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "aarch64-pa-forwardcfi"

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class PartsPassPointerLoadStore : public MachineFunctionPass {
 public:
   static char ID;

   PartsPassPointerLoadStore() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return "pauth-sllow"; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

   bool instrumentBranches(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   bool instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, type_id_t type_id);
   bool instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, type_id_t type_id);

 private:

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;
 };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PaForwardCfiPass() {
  return new PartsPassPointerLoadStore();
}

char PartsPassPointerLoadStore::ID = 0;

bool PartsPassPointerLoadStore::doInitialization(Module &M) {
  return false;
}

bool PartsPassPointerLoadStore::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
  DEBUG_PA_MIR(&MF, errs() << KBLU << "function " << MF.getName() << '\n' << KNRM);

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

  auto &C = MF.getFunction().getContext();

  for (auto &MBB : MF) {
    DEBUG_PA_MIR(&MF, do { errs() << KBLU << "\tBlock "; errs().write_escaped(MBB.getName()); } while(0));

    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      DEBUG_PA_OPT(&MF, do { errs() << "\t\t"; MIi->dump(); } while(false));

      const auto MIOpcode = MIi->getOpcode();
      auto partsType = PartsTypeMetadata::retrieve(*MIi);

      if (MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL) {
        instrumentBranches(MBB, MIi);
      } else if (partsUtils->isLoadOrStore(*MIi)) {
        /* ----------------------------- LOAD/STORE ---------------------------------------- */
        DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tfound a load/store (" << TII->getName(MIOpcode) << ")\n" << KNRM);

        if (partsType == nullptr) {
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to figure out type_id\n");
          auto Op = MIi->getOperand(0);
          const auto targetReg = Op.getReg();

          if (!partsUtils->checkIfRegInstrumentable(targetReg)) {
            partsType = PartsTypeMetadata::getIgnored();
          } else {
            if (partsUtils->isStore(*MIi)) {
              partsType = partsUtils->inferPauthTypeIdRegBackwards(MF, MBB, *MIi, targetReg);
            } else {
              // FIXME: this only supports loads of type load reg [reg, imm]
              if (MIi->getOperand(2).isImm()) {
                partsType = partsUtils->inferPauthTypeIdStackBackwards(MF, MBB, *MIi, targetReg, MIi->getOperand(1).getReg(), MIi->getOperand(2).getImm());
              } else {
                DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tOMG! unexpected operands, is this a pair store thingy?\n");
                partsType = PartsTypeMetadata::getUnknown();
              }
            }
          }
          partsUtils->attach(MF.getFunction().getContext(), partsType, &*MIi);

          MIi->addOperand(MachineOperand::CreateMetadata(partsType->getMDNode(C)));
          DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tstoring type_id (" << partsType->getTypeId() << ") in current MI\n" << KNRM);
        }

        if (!partsType->isKnown()) {
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\ttype_id is unknown!\n");
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
        const auto modReg = PARTS::getModifierReg();
        const auto type_id = partsType->getTypeId();

        if (partsUtils->isStore(*MIi)) {
          if (partsType->isDataPointer())
            partsUtils->pacDataPointer(MBB, MIi, reg, modReg, type_id);
          else
            partsUtils->pacCodePointer(MBB, MIi, reg, modReg, type_id);
        } else {
          auto loc = MIi;
          loc++;
          if (partsType->isDataPointer())
            partsUtils->autDataPointer(MBB, loc, reg, modReg, type_id);
          else
            partsUtils->autCodePointer(MBB, loc, reg, modReg, type_id);
        }
      }
    }
  }

  return true;
}

bool PartsPassPointerLoadStore::instrumentBranches(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();
  auto partsType = PartsTypeMetadata::retrieve(*MIi);

  assert(MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL);

  /* ----------------------------- BL/BLR ---------------------------------------- */
  DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tfound a BL/BLR (" << TII->getName(MIOpcode) << ")\n");

  if (partsType == nullptr) {
    DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\ttrying to figure out type_id\n");
    DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tUNIMPLEMENTED!!!\n");

    partsType = PartsTypeMetadata::getUnknown();
  }

  if (!partsType->isKnown()) {
    DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\ttype_id is unknown!\n");
    return false;
  }

  if (partsType->isIgnored()) {
    DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tmarked as ignored, skipping\n" << KNRM);
    return false;
  }

  if (!partsType->isPointer()) {
    DEBUG_PA_MIR(&MF, errs() << KCYN << "\t\t\tnot a pointer, skipping\n" << KNRM);
    return false;
  }

  DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\t going to instrument indirect call (type_id=" << partsType->getTypeId() << ")\n");

  assert(MIOpcode != AArch64::BL && "Whoops, thought this was never, maybe, gonna happen. I guess?");

  // Create the PAC modifier
  partsUtils->moveTypeIdToReg(MBB, MIi, PARTS::getModifierReg(), partsType->getTypeId());
  /*
  BuildMI(MBB, *MIi, DebugLoc(), TII->get(AArch64::MOVZXi))
      .addReg(PARTS::getModifierReg())
      .addImm(partsType->getTypeId())
      .addImm(0);
  */

  // Swap out the branch to a auth+branch variant
  auto BMI = BuildMI(MBB, *MIi, MIi->getDebugLoc(), TII->get(AArch64::BLRAA));
  BMI.add(MIi->getOperand(0));
  BMI.addReg(PARTS::getModifierReg());

  // Remove the old instruction!
  auto &MI = *MIi;
  MIi--;
  MI.removeFromParent();

  return true;
}


bool PartsPassPointerLoadStore::instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI,
                                                           unsigned pointerReg, type_id_t type_id)
{
  return false;
}

bool PartsPassPointerLoadStore::instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI,
                                                          unsigned pointerReg, type_id_t type_id)
{
  return false;
}
