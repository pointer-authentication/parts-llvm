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
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "aarch64-parts-pass-cpi"

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsPassCpi : public MachineFunctionPass {

 public:
   static char ID;


   AArch64PartsPassCpi() :
   MachineFunctionPass(ID),
   log(PARTS::PartsLog::getLogger(DEBUG_TYPE))
   {
     DEBUG_PA(log->enable());
   }

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:

   PartsLog_ptr log;

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;

   Function *funcCountCodePtrBranch = nullptr;
   Function *funcCountCodePtrCreate = nullptr;
 };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsPassCpi() {
  return new AArch64PartsPassCpi();
}

char AArch64PartsPassCpi::ID = 0;

bool AArch64PartsPassCpi::doInitialization(Module &M) {
  funcCountCodePtrBranch = PartsEventCount::getFuncCodePointerBranch(M);
  funcCountCodePtrCreate = PartsEventCount::getFuncCodePointerCreate(M);
  return true;
}

bool AArch64PartsPassCpi::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;
  DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
  DEBUG_PA(log->debug() << "function " << MF.getName() << "\n");

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

  for (auto &MBB : MF) {
    DEBUG_PA(log->debug(MF.getName()) << "  block " << MBB.getName() << "\n");

    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      DEBUG_PA(log->debug(MF.getName()) << "   " << MIi);

      const auto MIOpcode = MIi->getOpcode();

      if (MIOpcode == AArch64::PARTS_PACIA) {
        log->inc(DEBUG_TYPE ".pacia", true) << "converting PARTS_PACIA\n";

        auto &MI = *MIi--;

        partsUtils->addEventCallFunction(MBB, MI, MIi->getDebugLoc(), funcCountCodePtrCreate);
        partsUtils->convertPartIntrinsic(MBB, MI, AArch64::PACIA);

        found = true; // make sure we return true when we modify stuff
      } else if (MIOpcode == AArch64::PARTS_AUTIA) {
        log->inc(DEBUG_TYPE ".autia", true) << "converting PARTS_AUTIA\n";

        auto &MI_autia = *MIi;
        MIi--; // move iterator back since we're gonna change latter stuff

        // Try to find the corresponding BLR
        MachineInstr *MI_blr = &MI_autia;
        do {
          MI_blr = MI_blr->getNextNode();
          if (MI_blr == nullptr) {
            MBB.dump(); // dump for debugging...
            llvm_unreachable("failed to find BLR for AUTIA");
          }
        } while (MI_blr->getOpcode() != AArch64::BLR);

        // This is where the instrumentation "happens"
        auto *loc = MI_blr;
        const auto DL = loc->getDebugLoc();
        // But this is the IR intrinsic that has the needed info!
        const unsigned mod = MI_autia.getOperand(2).getReg();
        const unsigned src = MI_autia.getOperand(1).getReg();
        const unsigned dst = MI_autia.getOperand(0).getReg(); // This goes to waste

        if (!PARTS::useDummy()) {
          auto BMI = BuildMI(MBB, loc, DL, TII->get(AArch64::BLRAA));
          BMI.addReg(src);
          BMI.addReg(mod);
        } else {
          // FIXME: This might break if the pointer is reused elsewhere!!!
          partsUtils->addNops(MBB, loc, src, mod, DL);
        }

        // Remove the old instructions!
        MI_autia.removeFromParent();
        MI_blr->removeFromParent();
      }
    }
  }

  return found;
}

