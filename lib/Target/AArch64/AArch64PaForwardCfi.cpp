/*
 * AArch64PaForwardCfi.cpp
 * Copyright (C) 2018 Secure Systems Group, Aalto University, ssg.aalto.fi
 * Author: Hans Liljestrand <liljestrandh@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */


#include <iostream>
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

   bool instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg);
   bool instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg);

 private:
   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
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

  for (auto &MBB : MF) {
    DEBUG_PA_MIR(&MF, errs() << KBLU << "\tBlock ");
    DEBUG_PA_MIR(&MF, errs().write_escaped(MBB.getName()) << KNRM << "\n");

    for (auto &MI : MBB) {
      DEBUG_PA_MIR(&MF, errs() << "\t\t");
      DEBUG_PA_MIR(&MF, MI.dump());

      if (PA::isLoad(MI) || PA::isStore(MI)) {
        DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tfound a load/store (" << TII->getName(MI.getOpcode()) << ")\n" << KNRM);

        auto PAMDNode = getPAData(MI);

        if (PAMDNode == nullptr) {
          // Type is unknown, we will need to figure out what to do by looking around!
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tno PAData!\n" << KNRM);
          // TODO: check if possible pointer!
          // TODO: traverse iterator to find use-case with metadata

          continue;
        }

        const auto type_id = getPauthTypeId(getPAData(MI));

        if (! isPointer(type_id)) {
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tnot a pointer!\n" << KNRM);
          continue;
        }

        if (isInstruction(type_id)) {
          // We will eventually skip these and assume they are always PACed!
        }

        if (isInstruction(type_id)) {
          DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tis a instruction pointer!\n" << KNRM);

          if (PA::isLoad(MI) || PA::isStore(MI)) {
            DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\t skipping\n");
          }
        } else {
          DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tis a instruction pointer!\n" << KNRM);

          if (PA::isStore(MI)) {
            DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tinstrumenting store\n");
            instrumentDataPointerStore(MBB, MI, MI.getOperand(0).getReg());
          } else {
            DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tinstrumenting load\n");
            instrumentDataPointerLoad(MBB, MI, MI.getOperand(0).getReg());
          }
        }

        continue;
      }

      // TODO: handle other stuff...
    }
  }

  return true;
}

bool PaForwardCfi::instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg)
{
  const auto  PAOpcode = AArch64::PACDA;

  BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::MOVZXi)).addReg(Pauth_ModifierReg).addImm(0).addImm(0);
  BuildMI(MBB, MI, DebugLoc(), TII->get(PAOpcode)).addReg(pointerReg).addReg(Pauth_ModifierReg);

  return true;
}

bool PaForwardCfi::instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg)
{
  const auto  PAOpcode = AArch64::AUTDA;
  auto iter = MI.getIterator();
  iter++;

  BuildMI(MBB, iter, DebugLoc(), TII->get(AArch64::MOVZXi)).addReg(Pauth_ModifierReg).addImm(0).addImm(0);
  BuildMI(MBB, iter, DebugLoc(), TII->get(PAOpcode)).addReg(pointerReg).addReg(Pauth_ModifierReg);

  return true;
}


