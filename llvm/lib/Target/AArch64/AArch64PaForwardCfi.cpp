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

        const auto type_id = getPauthType(getPAData(MI));

        if (! isPointer(type_id)) {
          DEBUG_PA_MIR(&MF, errs() << KRED << "\t\t\tnot a pointer!\n" << KNRM);
          continue;
        }

        if (isInstruction(type_id)) {
          DEBUG_PA_MIR(&MF, errs() << KBLU << "\t\t\tis a instruction pointer!\n" << KNRM);
          // We will eventually skip these and assume they are always PACed!
        }

        if (PA::isLoad(MI)) {
          auto PAOpcode = isInstruction(type_id) ? AArch64::AUTIA : AArch64::AUTDA;
          DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tadding " << TII->getName(PAOpcode) << " instruction\n");

          auto iter = MI.getIterator();
          iter++;

          auto tmpReg = AArch64::X23;
          BuildMI(MBB, iter, DebugLoc(), TII->get(AArch64::MOVZWi)).addReg(tmpReg).addImm(0).addImm(0);
          BuildMI(MBB, iter, DebugLoc(), TII->get(PAOpcode)).addReg(MI.getOperand(0).getReg()).addReg(tmpReg);
        } else {
          auto PAOpcode = isInstruction(type_id) ? AArch64::PACIA : AArch64::PACDA;
          DEBUG_PA_MIR(&MF, errs() << KGRN << "\t\t\tadding " << TII->getName(PAOpcode) << " instruction\n");

          auto tmpReg = AArch64::X23;
          BuildMI(MBB, MI, DebugLoc(), TII->get(AArch64::MOVZWi)).addReg(tmpReg).addImm(0) .addImm(0);
          BuildMI(MBB, MI, DebugLoc(), TII->get(PAOpcode)).addReg(MI.getOperand(0).getReg()).addReg(tmpReg);
        }

        continue;
      }

      // TODO: handle other stuff...
    }
  }

  return true;
}
