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
#include "llvm/CodeGen/RegisterScavenging.h"
// PARTS includes
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/PARTS/PartsLog.h"
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsCpiPass"

STATISTIC(StatAutia, DEBUG_TYPE ": code pointers authenticated and unPACed");

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

 private:

   PartsLog_ptr log;

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;

   Function *funcCountCodePtrBranch = nullptr;
   Function *funcCountCodePtrCreate = nullptr;

   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline bool LowerPARTSPACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline bool LowerPARTSAUTCALL(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline bool LowerPARTSAUTIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline MachineInstr *FindIndirectCallMachineInstr(MachineInstr *MI);
   inline bool isIndirectCall(const MachineInstr &MI) const;
   inline void InsertAuthenticateBranchInstr(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dstReg, unsigned modReg, const MCInstrDesc &InstrDesc);
   inline void InsertMoveDstAddress(MachineBasicBlock &MBB, MachineInstr *MI_autia, unsigned dstReg, unsigned srcReg, const MCInstrDesc &InstrDesc);
   inline bool isNormalIndirectCall(const MachineInstr *MI) const;

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

    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie; ++MIi) {
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
  bool res = false;

  if (MIOpcode == AArch64::PARTS_PACIA)
    res = LowerPARTSPACIA(MF, MBB, MIi);
  else if (MIOpcode == AArch64::PARTS_AUTCALL)
    res = LowerPARTSAUTCALL(MF, MBB, MIi);
  else if (MIOpcode == AArch64::PARTS_AUTIA)
    res = LowerPARTSAUTIA(MF, MBB, MIi);

  return res;
}

inline bool AArch64PartsCpiPass::LowerPARTSPACIA( MachineFunction &MF,
                                                  MachineBasicBlock &MBB,
                                                  MachineBasicBlock::instr_iterator &MIi) {
    auto &MI = *MIi--;

    log->inc(DEBUG_TYPE ".pacia", true) << "converting PARTS_PACIA\n";

    partsUtils->addEventCallFunction(MBB, MI, MIi->getDebugLoc(), funcCountCodePtrCreate);
    partsUtils->convertPartIntrinsic(MBB, MI, AArch64::PACIA);

    return true;
}

inline bool AArch64PartsCpiPass::LowerPARTSAUTCALL( MachineFunction &MF,
                                                  MachineBasicBlock &MBB,
                                                  MachineBasicBlock::instr_iterator &MIi) {
  log->inc(DEBUG_TYPE ".autia", true) << "converting PARTS_AUTCALL\n";

  MachineInstr &MI_autia = *MIi;
  MIi--; // move iterator back since we're gonna change latter stuff

  const unsigned mod2 = MI_autia.getOperand(2).getReg();
  const unsigned src = MI_autia.getOperand(1).getReg();
  const unsigned dst = MI_autia.getOperand(0).getReg(); // unused!

  RegScavenger RS;
  RS.enterBasicBlockEnd(MBB);
  auto &RC = AArch64::GPR64commonRegClass;

  MachineInstr *MI_indcall = FindIndirectCallMachineInstr(MI_autia.getNextNode());
  if (MI_indcall == nullptr) {
      // This shouldn't happen, as it indicates that we didn't find what we were looking for
      // and have an orphaned pacia.
      DEBUG(MBB.dump()); // dump for debugging...
      llvm_unreachable("failed to find BLR for AUTCALL");
    }

  RS.backward(--MachineBasicBlock::iterator(MI_indcall));
  const unsigned mod = RS.scavengeRegisterBackwards(RC, MachineBasicBlock::iterator(MI_autia), false, 0);
  RS.setRegUsed(mod); // Tell the Register Scavenger that the register is alive.
  InsertMoveDstAddress(MBB, &MI_autia, mod, mod2, TII->get(AArch64::ORRXrs));
  InsertMoveDstAddress(MBB, &MI_autia, dst, src, TII->get(AArch64::ORRXrs));

  const auto DL = MI_indcall->getDebugLoc();
  partsUtils->addEventCallFunction(MBB, *MIi, DL, funcCountCodePtrBranch);

  if (PARTS::useDummy()) {
    // FIXME: This might break if the pointer is reused elsewhere!!!
    partsUtils->addNops(MBB, MI_indcall, src, mod, DL);
  } else {
    if (isNormalIndirectCall(MI_indcall)) {
      InsertAuthenticateBranchInstr(MBB, MI_indcall, dst, mod, TII->get(AArch64::BLRAA));
    } else {
   // This is a tail call return, and we need to use BRAA
      // (tail-call: ~optimizatoin where a tail-cal is converted to a direct call so that
      //  the tail-called function can return immediately to the current callee, without
      //  going through the currently active function.)
      InsertAuthenticateBranchInstr(MBB, MI_indcall, dst, mod, TII->get(AArch64::BRAA));
    }

    // Remove the replaced BR instruction
    MI_indcall->removeFromParent();
  }

  // Remove the PARTS intrinsic!
  MI_autia.removeFromParent();

  return true;
}

inline bool AArch64PartsCpiPass::LowerPARTSAUTIA( MachineFunction &MF,
                                                  MachineBasicBlock &MBB,
                                                  MachineBasicBlock::instr_iterator &MIi) {
  log->inc(DEBUG_TYPE ".autia", true) << "converting PARTS_AUTIA\n";

  MachineInstr &MI_autia = *MIi;
  MIi--; // move iterator back since we're gonna change latter stuff

  const unsigned mod2 = MI_autia.getOperand(2).getReg();
  const unsigned src = MI_autia.getOperand(1).getReg();
  const unsigned dst = MI_autia.getOperand(0).getReg(); // unused!

  RegScavenger RS;
  RS.enterBasicBlockEnd(MBB);
  auto &RC = AArch64::GPR64commonRegClass;
  MachineFrameInfo &MFI = MF.getFrameInfo();
  unsigned Size = TRI->getSpillSize(RC);
  unsigned Align = TRI->getSpillAlignment(RC);
  RS.addScavengingFrameIndex(MFI.CreateSpillStackObject(Size, Align)); // Add an emergency spill slot in the stack
  RS.backward(); /* Find a reg that it is not used by last instruction and in case of spill restore before that one.*/
  const unsigned mod = RS.scavengeRegisterBackwards(RC, MachineBasicBlock::iterator(MI_autia), false, 0);
  RS.setRegUsed(mod); // Tell the Register Scavenger that the register is alive.
  InsertMoveDstAddress(MBB, &MI_autia, mod, mod2, TII->get(AArch64::ORRXrs));

      BuildMI(MBB, MI_autia, MI_autia.getDebugLoc(), TII->get(AArch64::AUTIA))
          .addUse(dst)
          .addUse(mod);

  ++StatAutia;
   // Remove the PARTS intrinsic!
  MI_autia.removeFromParent();

  return true;
}


inline MachineInstr *AArch64PartsCpiPass::FindIndirectCallMachineInstr(MachineInstr *MI) {
  while (MI != nullptr && !isIndirectCall(*MI))
    MI = MI->getNextNode();

  return MI;
}

inline bool AArch64PartsCpiPass::isIndirectCall(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
    case AArch64::BLR:        // Normal indirect call
    case AArch64::TCRETURNri: // Indirect tail call
      return true;
  }
  return false;
}

inline void AArch64PartsCpiPass::InsertAuthenticateBranchInstr(MachineBasicBlock &MBB,
                                                                MachineInstr *MI_indcall,
                                                                unsigned dstReg,
                                                                unsigned modReg,
                                                                const MCInstrDesc &InstrDesc) {
      auto BMI = BuildMI(MBB, MI_indcall, MI_indcall->getDebugLoc(), InstrDesc);
      BMI.addUse(dstReg);
      BMI.addUse(modReg);
}

inline void AArch64PartsCpiPass::InsertMoveDstAddress(MachineBasicBlock &MBB,
                                                                MachineInstr *MI_autia,
                                                                unsigned dstReg,
                                                                unsigned srcReg,
                                                                const MCInstrDesc &InstrDesc) {
  auto MOVMI = BuildMI(MBB, MI_autia, MI_autia->getDebugLoc(), InstrDesc, dstReg);
  MOVMI.addUse(AArch64::XZR);
  MOVMI.addUse(srcReg);
  MOVMI.addImm(0);
}

inline bool AArch64PartsCpiPass::isNormalIndirectCall(const MachineInstr *MI) const {
  return MI->getOpcode() == AArch64::BLR;
}

