//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
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
#include "llvm/ADT/Statistic.h"
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
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "AArch64PartsPass.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsDpiPass"

STATISTIC(StatStoreLoadInferred, "store load inferred");
STATISTIC(StatDataStore, "instrumented data stores");
STATISTIC(StatDataLoad, "instrumented data loads");

using namespace llvm;
using namespace llvm::PARTS;

#define skipIfB(ifx, stat, b, string) do {  \
    if ((ifx)) {                            \
      return false;                         \
    }                                       \
} while(false)

#define skipIfN(ifx, stat, string) do {     \
    if ((ifx)) {                            \
      return false;                         \
    }                                       \
} while (false)

namespace {
 class AArch64PartsDpiPass : public MachineFunctionPass, private AArch64PartsPass {

 public:
   static char ID;

   AArch64PartsDpiPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;
   bool instrumentLoadStore(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);

 private:

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;
   int m_PACed_me_a_live_one = false; // FIXME: horrible hack!
   MachineOperand *m_the_live_one = nullptr;

   Function *funcCountDataStr = nullptr;
   Function *funcCountDataLdr = nullptr;
 };
} // end anonymous namespace

FunctionPass *llvm::createPartsPassDpi() {
  return new AArch64PartsDpiPass();
}

char AArch64PartsDpiPass::ID = 0;

bool AArch64PartsDpiPass::doInitialization(Module &M) {
  funcCountDataStr = PartsEventCount::getFuncDataStr(M);
  funcCountDataLdr = PartsEventCount::getFuncDataLdr(M);
  return true;
}

bool AArch64PartsDpiPass::runOnMachineFunction(MachineFunction &MF) {
  if (hasNoPartsAttribute(MF))
    return false;

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();

  partsUtils = PartsUtils::get(TRI, TII);

  for (auto &MBB : MF) {
    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {

      if (partsUtils->isLoadOrStore(*MIi)) {
        instrumentLoadStore(MF, MBB, MIi);
      }

      if (m_PACed_me_a_live_one > 1) {
        // llvm_unreachable("didn't immediately get rid of un-killed PACed store");
        m_PACed_me_a_live_one = 0;
        m_the_live_one = nullptr;
      } else if (m_PACed_me_a_live_one > 0) {
        m_PACed_me_a_live_one++;
      }
    }
  }

  return true;
}

bool AArch64PartsDpiPass::instrumentLoadStore(MachineFunction &MF, MachineBasicBlock &MBB,
                                         MachineBasicBlock::instr_iterator &MIi) {
  assert(partsUtils->isLoadOrStore(*MIi));

  auto partsType = PartsTypeMetadata::retrieve(*MIi);
  auto &C = MF.getFunction().getContext();

  const auto MIOpcode = MIi->getOpcode();
  const auto MIName = TII->getName(MIOpcode).str();


  if (partsType == nullptr) {
    auto Op = MIi->getOperand(0);
    const auto targetReg = Op.getReg();

    //if (!partsUtils->checkIfRegInstrumentable(targetReg)) {
    if (!TRI->getPointerRegClass(MF)->contains(Op.getReg())) {
      partsType = PartsTypeMetadata::getIgnored();
      // Just to make sure this behaves as expected...
      assert(Op.getReg() != AArch64::X0 && Op.getReg() != AArch64::X1 && Op.getReg() != AArch64::X2 &&
             Op.getReg() != AArch64::X3 && Op.getReg() != AArch64::X4 && Op.getReg() != AArch64::X5 &&
             Op.getReg() != AArch64::X6 && Op.getReg() != AArch64::X7 && Op.getReg() != AArch64::X8 &&
             Op.getReg() != AArch64::X9 && Op.getReg() != AArch64::X11 && Op.getReg() != AArch64::X12 &&
             Op.getReg() != AArch64::X13 && Op.getReg() != AArch64::X14 && Op.getReg() != AArch64::X15 &&
             Op.getReg() != AArch64::X16 && Op.getReg() != AArch64::X17 && Op.getReg() != AArch64::X18 &&
             Op.getReg() != AArch64::X19 && Op.getReg() != AArch64::X21 && Op.getReg() != AArch64::X22 &&
             Op.getReg() != AArch64::X23 && Op.getReg() != AArch64::X24 && Op.getReg() != AArch64::X25 &&
             Op.getReg() != AArch64::X26 && Op.getReg() != AArch64::X27 && Op.getReg() != AArch64::X28 &&
             Op.getReg() != AArch64::FP && Op.getReg() != AArch64::LR);
    } else if (Op.getReg() == AArch64::FP || Op.getReg() == AArch64::LR) {
      // Ignore FP and LR, they are handled elsewhre
      partsType = PartsTypeMetadata::getIgnored();
    } else {
      // remove this call at some point, checkIfRegInstrumentable is crappy...
      assert(partsUtils->checkIfRegInstrumentable(targetReg));

      if (partsUtils->isStore(*MIi)) {
        partsType = partsUtils->inferPauthTypeIdRegBackwards(MF, MBB, *MIi, targetReg);
      } else {
        // FIXME: this only supports loads of type load reg [reg, imm]
        if (MIi->getOperand(2).isImm()) {
          partsType = partsUtils->inferPauthTypeIdStackBackwards(MF, MBB, *MIi, targetReg,
                                                                 MIi->getOperand(1).getReg(),
                                                                 MIi->getOperand(2).getImm());
        } else {
          DEBUG(errs() << __FUNCTION__ << ": OMG! unexpected operands, is this a pair store thingy?\n");
          partsType = PartsTypeMetadata::getUnknown();
        }
      }
    }
    partsUtils->attach(MF.getFunction().getContext(), partsType, &*MIi);

    MIi->addOperand(MachineOperand::CreateMetadata(partsType->getMDNode(C)));
    ++StatStoreLoadInferred;
  }

  skipIfN(partsType->isIgnored(), DEBUG_TYPE ".StoreLoad.Ignored_" + MIName, "marked as ignored, skipping!\n");

  if (!partsType->isKnown() && (
      MIi->getFlag(MachineInstr::MIFlag::FrameSetup) ||
      MIi->getFlag(MachineInstr::MIFlag::FrameDestroy))) {
    // We're assuming this is a callee saved registerThing, so therefore this op should be relative to SP...
    assert(MIi->getOperand(1).getReg() == AArch64::SP || MIi->getOperand(2).getReg() == AArch64::SP);
    return false;
  }

  if (!partsType->isKnown()) {
    errs() << "___________________________________________________________________\n";
    errs() << MF.getFunction().getName() << " : " << MBB.getName() << "\n";
    errs() << MIi->getFlag(MachineInstr::MIFlag::FrameSetup) << "\n";
    errs() << MIi->getFlag(MachineInstr::MIFlag::FrameDestroy) << "\n";
    DEBUG(MIi->dump());
    errs() << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
  }

  skipIfB(!partsType->isKnown(), DEBUG_TYPE ".StoreLoad.Unknown_" + MIName, false, "type_id is unknown!\n");
  skipIfN(!partsType->isPointer(), DEBUG_TYPE ".StoreLoad.NotAPointer_" + MIName, "not a pointer, skipping!\n");
  skipIfN(partsType->isCodePointer(), DEBUG_TYPE ".StoreLoad.IgnoringCodePointer_" + MIName, "ignoring code pointer\n");

  auto reg = MIi->getOperand(0).getReg();
  const auto modReg = PARTS::getModifierReg();
  const auto type_id = partsType->getTypeId();

  if (partsUtils->isStore(*MIi)) {
    if (PARTS::useDpi()) {
      ++StatDataStore;

      // FIXME: Horrible hack for double define!
      if (m_PACed_me_a_live_one == 0) {
        const auto &DL = MIi->getDebugLoc();
        partsUtils->pacDataPointer(MBB, MIi, reg, modReg, type_id, DL);
        partsUtils->addEventCallFunction(MBB, *MIi, DL, funcCountDataStr);
      } else {
        assert(m_the_live_one->getReg() == MIi->getOperand(0).getReg());
      }

      if (MIi->getOperand(0).isKill()) {
        m_PACed_me_a_live_one = 0;
      } else {
        m_PACed_me_a_live_one = 1;
        m_the_live_one = &(MIi->getOperand(0));
      }

      return true;
    }
  } else {
    auto loc = MIi;
    loc++;
    if (PARTS::useDpi()) {
      ++StatDataLoad;

      const auto &DL = loc->getDebugLoc();
      partsUtils->autDataPointer(MBB, loc, reg, modReg, type_id, DL);
      partsUtils->addEventCallFunction(MBB, *MIi, DL, funcCountDataLdr);
      return true;
    }
  }

  return false;
}
