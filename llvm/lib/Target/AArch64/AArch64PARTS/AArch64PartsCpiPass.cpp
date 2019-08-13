//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
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
#include "llvm/CodeGen/RegisterScavenging.h"
// PARTS includes
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsCpiPass"

STATISTIC(StatAutia, DEBUG_TYPE ": code pointers authenticated and unPACed");
STATISTIC(StatPacia, DEBUG_TYPE ": code pointers signed");

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsCpiPass : public MachineFunctionPass {

 public:
   static char ID;

   AArch64PartsCpiPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
   const AArch64InstrInfo *TII = nullptr;

   void lowerPARTSAUTCALL(MachineBasicBlock &MBB, MachineInstr &MI);
   void lowerPARTSPACIA(MachineBasicBlock &MBB, MachineInstr &MI);

   inline bool handleInstruction(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline void lowerPARTSAUTIA(MachineBasicBlock &MBB, MachineInstr &MI);
   void lowerPARTSIntrinsicCommon(MachineBasicBlock &MBB, MachineInstr &MI, const MCInstrDesc &InstrDesc);
   inline bool isPartsIntrinsic(unsigned Opcode);
 };

} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsPassCpi() {
  return new AArch64PartsCpiPass();
}

char AArch64PartsCpiPass::ID = 0;

bool AArch64PartsCpiPass::doInitialization(Module &M) {
  return true;
}

bool AArch64PartsCpiPass::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;

  TII = MF.getSubtarget<AArch64Subtarget>().getInstrInfo();

  for (auto &MBB : MF)
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie; ++MIi)
      found = handleInstruction(MBB, MIi) || found;

  return found;
}

/**
 *
 * @param MBB
 * @param MIi
 * @return  return true when changing something, otherwise false
 */
inline bool AArch64PartsCpiPass::handleInstruction(MachineBasicBlock &MBB,
                                                   MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();

  if (!isPartsIntrinsic(MIOpcode))
    return false;

  auto &MI = *MIi--;

  switch (MIOpcode) {
    default:
      llvm_unreachable("Unhandled PARTS intrinsic!!");
    case AArch64::PARTS_PACIA:
      lowerPARTSPACIA(MBB, MI);
      break;
    case AArch64::PARTS_AUTIA:
      lowerPARTSAUTIA(MBB, MI);
      break;
    case AArch64::PARTS_AUTCALL:
      lowerPARTSAUTCALL(MBB, MI);
      break;
  }

  MI.removeFromParent(); // Remove the PARTS intrinsic!

  return true;
}

inline bool AArch64PartsCpiPass::isPartsIntrinsic(unsigned Opcode) {
  switch (Opcode) {
    case AArch64::PARTS_PACIA:
    case AArch64::PARTS_AUTIA:
    case AArch64::PARTS_AUTCALL:
      return true;
  }

  return false;
}
void AArch64PartsCpiPass::lowerPARTSPACIA(MachineBasicBlock &MBB,
                                          MachineInstr &MI) {
  lowerPARTSIntrinsicCommon(MBB, MI, TII->get(AArch64::PACIA));
  ++StatPacia;
}

void AArch64PartsCpiPass::lowerPARTSAUTCALL(MachineBasicBlock &MBB,
                                            MachineInstr &MI) {

  LLVM_DEBUG(MBB.dump());
  llvm_unreachable("Unexpected AUTCALL found !!!!");
}

inline void AArch64PartsCpiPass::lowerPARTSAUTIA(MachineBasicBlock &MBB,
                                                 MachineInstr &MI) {
  lowerPARTSIntrinsicCommon(MBB, MI, TII->get(AArch64::AUTIA));
  ++StatAutia;
}

void AArch64PartsCpiPass::lowerPARTSIntrinsicCommon(MachineBasicBlock &MBB,
                                                    MachineInstr &MI,
                                                    const MCInstrDesc &InstrDesc) {
  auto &mod = MI.getOperand(2);
  auto &dst = MI.getOperand(0);
  auto BMI = BuildMI(MBB, MI, MI.getDebugLoc(), InstrDesc);
  BMI.add(dst);
  BMI.add(mod);
}
