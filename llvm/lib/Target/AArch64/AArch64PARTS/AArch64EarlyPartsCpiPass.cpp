//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
//         Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
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
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64EarlyPartsCpiPass"

STATISTIC(StatAutcall, DEBUG_TYPE ": inserted authenticate and branch instructions");

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
   void triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB);
   inline bool isIndirectCall(const MachineInstr &MI) const;
   inline const MCInstrDesc &getIndirectCallAuth(MachineInstr *MI_indcall);
   inline void replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB, MachineInstr *MI_indcall, MachineInstr &MI);
   inline void insertCOPYInstr(MachineBasicBlock &MBB, MachineInstr *MI_indcall, MachineInstr &MI);
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

  auto &MI = *MIi--;
  replaceBranchByAuthenticatedBranch(MBB, MI_indcall, MI);
  ++StatAutcall;

  return true;
}

inline const MCInstrDesc &AArch64EarlyPartsCpiPass::getIndirectCallAuth(MachineInstr *MI_indcall) {

  if (MI_indcall->getOpcode() == AArch64::BLR)
   return TII->get(AArch64::BLRAA);

  // This is a tail call return, and we need to use BRAA
  // (tail-call: ~optimizatoin where a tail-cal is converted to a direct call so that
  // the tail-called function can return immediately to the current callee, without
  // going through the currently active function.)

 return TII->get(AArch64::TCRETURNriAA);
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

void AArch64EarlyPartsCpiPass::triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB) {
  LLVM_DEBUG(MBB.dump());
  llvm_unreachable("failed to find BLR for AUTCALL");
}

inline void AArch64EarlyPartsCpiPass::replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB,
                                                                    MachineInstr *MI_indcall,
                                                                    MachineInstr &MI) {
  auto modOperand = MI.getOperand(2);

  insertCOPYInstr(MBB, MI_indcall, MI);

  auto BMI = BuildMI(MBB, *MI_indcall, MI_indcall->getDebugLoc(), getIndirectCallAuth(MI_indcall));
  BMI.addUse(MI_indcall->getOperand(0).getReg());
  if (MI_indcall->getOpcode() == AArch64::TCRETURNri)
    BMI.add(MI_indcall->getOperand(1)); // Copy FPDiff from original tail call pseudo instruction
  BMI.add(modOperand);
  BMI.copyImplicitOps(*MI_indcall);

  MI_indcall->removeFromParent();
  MI.removeFromParent();
}

inline void AArch64EarlyPartsCpiPass::insertCOPYInstr(MachineBasicBlock &MBB,
                                                      MachineInstr *MI_indcall,
                                                      MachineInstr &MI) {

  auto dstOperand = MI.getOperand(0);
  auto srcOperand = MI.getOperand(1);

  auto COPYMI = BuildMI(MBB, *MI_indcall, MI_indcall->getDebugLoc(), TII->get(AArch64::COPY));
  COPYMI.add(dstOperand);
  COPYMI.add(srcOperand);
}
