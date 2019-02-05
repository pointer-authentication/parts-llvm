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

#define DEBUG_TYPE "AArch64PartsCpiPass"

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsCpiPass : public MachineFunctionPass {

 public:
   static char ID;


   AArch64PartsCpiPass() :
   MachineFunctionPass(ID),
   log(PARTS::PartsLog::getLogger(DEBUG_TYPE))
   {
     DEBUG_PA(log->enable());
   }

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;
   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);

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
  return new AArch64PartsCpiPass();
}

char AArch64PartsCpiPass::ID = 0;

bool AArch64PartsCpiPass::doInitialization(Module &M) {
  funcCountCodePtrBranch = PartsEventCount::getFuncCodePointerBranch(M);
  funcCountCodePtrCreate = PartsEventCount::getFuncCodePointerCreate(M);
  return true;
}

bool AArch64PartsCpiPass::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;
  DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
  DEBUG_PA(log->debug() << "function " << MF.getName() << "\n");

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

  //MF.dump();
  for (auto &MBB : MF) {
    DEBUG_PA(log->debug(MF.getName()) << "  block " << MBB.getName() << "\n");

    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      DEBUG_PA(log->debug(MF.getName()) << "   " << MIi);

      found = handleInstruction(MF, MBB, MIi) || found;
    }
  }

  return found;
}


/**
 *
 * @param MF
 * @param MBB
 * @param MIi
 * @return  return true when changing something, otherwise false
 */
inline bool AArch64PartsCpiPass::handleInstruction(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();

  if (MIOpcode == AArch64::PARTS_PACIA) {
    log->inc(DEBUG_TYPE ".pacia", true) << "converting PARTS_PACIA\n";

    auto &MI = *MIi--;

    partsUtils->addEventCallFunction(MBB, MI, MIi->getDebugLoc(), funcCountCodePtrCreate);
    partsUtils->convertPartIntrinsic(MBB, MI, AArch64::PACIA);

    return true;
  }

  if (MIOpcode == AArch64::PARTS_AUTIA) {
    log->inc(DEBUG_TYPE ".autia", true) << "converting PARTS_AUTIA\n";

    auto &MI_autia = *MIi;
    MachineInstr *loc_mov = &*MIi;
    MIi--; // move iterator back since we're gonna change latter stuff

    const auto MOVDL = loc_mov->getDebugLoc();
    const unsigned mod_src = MI_autia.getOperand(2).getReg();
    const unsigned mod_dst = PARTS::getModifierReg();
    const unsigned src = MI_autia.getOperand(1).getReg();
    const unsigned dst = MI_autia.getOperand(0).getReg();

    if (mod_dst != mod_src)
      BuildMI(MBB, loc_mov, MOVDL, TII->get(AArch64::ORRXrs))
          .addUse(mod_dst)
          .addUse(AArch64::XZR)
          .addUse(mod_src)
          .addImm(0);
    if (dst != src)
      BuildMI(MBB, loc_mov, MOVDL, TII->get(AArch64::ORRXrs))
          .addUse(dst)
          .addUse(AArch64::XZR)
          .addUse(src)
          .addImm(0);

    // Try to find the corresponding BLR
    MachineInstr *MI_blr = &MI_autia;
    do {
      MI_blr = MI_blr->getNextNode();
      if (MI_blr == nullptr) {
        // This shouldn't happen, as it indicates that we didn't find what we were looking for
        // and have an orphaned pacia.
        DEBUG(MBB.dump()); // dump for debugging...
        llvm_unreachable("failed to find BLR for AUTIA");
      }
    } while (MI_blr->getOpcode() != AArch64::BLR &&
             MI_blr->getOpcode() != AArch64::TCRETURNdi &&
             MI_blr->getOpcode() != AArch64::TCRETURNri);


    auto *loc = MI_blr;
    const auto DL = loc->getDebugLoc();
    partsUtils->addEventCallFunction(MBB, *MIi, DL, funcCountCodePtrBranch);

    if (PARTS::useDummy()) {
      // FIXME: This might break if the pointer is reused elsewhere!!!
      partsUtils->addNops(MBB, loc, src, mod_src, DL);
    } else {
      if (MI_blr->getOpcode() == AArch64::BLR) {
        // Normal indirect call
        BuildMI(MBB, loc, DL, TII->get(AArch64::BLRAA))
            .addUse(dst)
            .addUse(mod_dst);
      } else {
        // This is a tail call return, and we need to use BRAA
        // (tail-call: ~optimizatoin where a tail-cal is converted to a direct call so that
        //  the tail-called function can return immediately to the current callee, without
        //  going through the currently active function.)
        BuildMI(MBB, loc, DL, TII->get(AArch64::BRAA))
            .addUse(dst)
            .addUse(mod_dst);
      }

      // Remove the replaced BR instruction
      MI_blr->removeFromParent();
    }

    // Remove the PARTS intrinsic!
    MI_autia.removeFromParent();

    return true;
  }

  return false;
}
