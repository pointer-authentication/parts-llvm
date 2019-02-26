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
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64EarlyPartsCpiPass"

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64EarlyPartsCpiPass : public MachineFunctionPass {

 public:
   static char ID;

   AArch64EarlyPartsCpiPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   virtual bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;

   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline MachineInstr *findIndirectCallMachineInstr(MachineInstr *MI);
   inline void triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB);
   inline bool isIndirectCall(const MachineInstr &MI) const;
   };
} // end anonymous namespace

FunctionPass *llvm::createAArch64EarlyPartsPassCpi() {

  return new AArch64EarlyPartsCpiPass();
}

char AArch64EarlyPartsCpiPass::ID = 0;

bool AArch64EarlyPartsCpiPass::doInitialization(Module &M) {
  return true;
}

bool AArch64EarlyPartsCpiPass::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;

  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();

  for (auto &MBB : MF)
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie; ++MIi)
      found = handleInstruction(MF, MBB, MIi) || found;

  return found;
}

/**
 *
 * @param MF
 * @param MBB
 * @param MIi
 * @return  return true when changing something, otherwise false
 */
inline bool AArch64EarlyPartsCpiPass::handleInstruction(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();

  if (MIOpcode != AArch64::PARTS_AUTCALL)
    return false;

  MachineInstr *MI_indcall = findIndirectCallMachineInstr(MIi->getNextNode());
  if (MI_indcall == nullptr)
    triggerCompilationErrorOrphanAUTCALL(MBB);

  if (MI_indcall->getOpcode() == AArch64::TCRETURNri) // TODO: Handle tailcall, we need dedicate pseudo instr (TCRETURNA[AB]ri)
    return false;
  auto &MI = *MIi--;
  auto modOperand = MI.getOperand(2);
  auto dstOperand = MI.getOperand(1);
  auto BMI = BuildMI(MBB, *MI_indcall, MI_indcall->getDebugLoc(), TII->get(AArch64::BLRAA));
  BMI.add(dstOperand);
  BMI.add(modOperand);
  BMI.copyImplicitOps(*MI_indcall);

  MI_indcall->removeFromParent();
  MI.removeFromParent();

  return true;
}

inline MachineInstr *AArch64EarlyPartsCpiPass::findIndirectCallMachineInstr(MachineInstr *MI) {
  while (MI != nullptr && !isIndirectCall(*MI))
    MI = MI->getNextNode();

  return MI;
}

inline bool AArch64EarlyPartsCpiPass::isIndirectCall(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
    case AArch64::BLR:        // Normal indirect call
    case AArch64::TCRETURNri: // Indirect tail call
      return true;
  }
  return false;
}

inline void AArch64EarlyPartsCpiPass::triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB) {
  DEBUG(MBB.dump());
  llvm_unreachable("failed to find BLR for AUTCALL");
}
