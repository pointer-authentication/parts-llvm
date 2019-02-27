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

#define DEBUG_TYPE "AArch64PartsEmulatedTimingPass"

STATISTIC(StatAutia, DEBUG_TYPE ": Num of emulated auti[ab] instructions");
STATISTIC(StatPacia, DEBUG_TYPE ": Num of emulated paci[ab] instructions");
STATISTIC(StatAuthBranch, DEBUG_TYPE " Num of emulated auti[ia] for branch instructions");

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsEmulatedTimingPass : public MachineFunctionPass {

 public:
   static char ID;

   AArch64PartsEmulatedTimingPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
  const AArch64Subtarget *STI = nullptr;
  const AArch64InstrInfo *TII = nullptr;
  inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
  inline void doMachineFunctionInit(MachineFunction &MF);
  inline bool isPAInstruction(unsigned Opcode);
  inline void addNops(MachineBasicBlock &MBB, MachineInstr *MI, unsigned ptrReg, unsigned modReg, const DebugLoc &DL);
  inline void replacePACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
  inline void replaceAUTIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
  inline void replaceBranchAuth(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
  inline const MCInstrDesc &getIndirectCallMachineInstruction(MachineInstr &MI);
  void replacePAInstructionCommon(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
 };

} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsEmulatedTimingPass() {
   return new AArch64PartsEmulatedTimingPass();
}

char AArch64PartsEmulatedTimingPass::ID = 0;

bool AArch64PartsEmulatedTimingPass::doInitialization(Module &M) {
  return true;
}

inline void AArch64PartsEmulatedTimingPass::doMachineFunctionInit(MachineFunction &MF) {
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
}

bool AArch64PartsEmulatedTimingPass::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;

  doMachineFunctionInit(MF);

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
inline bool AArch64PartsEmulatedTimingPass::handleInstruction(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();

  if (!isPAInstruction(MIOpcode))
    return false;
  auto &MI = *MIi--;

  switch (MIOpcode) {
    default:
      llvm_unreachable("Unhandled PAC instruction!!");
    case AArch64::PACIA:
      replacePACIA(MF, MBB, MI);
      break;
    case AArch64::AUTIA:
      replaceAUTIA(MF, MBB, MI);
      break;
    case AArch64::BLRAA:
    case AArch64::BRAA:
      replaceBranchAuth(MF, MBB, MI);
      break;
  }

  MI.removeFromParent(); // Remove pac instruction !
  MBB.dump();
  return true;
}

inline bool AArch64PartsEmulatedTimingPass::isPAInstruction(unsigned Opcode) {
  switch (Opcode) {
    case AArch64::PACIA:
    case AArch64::AUTIA:
    case AArch64::BLRAA:
    case AArch64::BRAA:
      return true;
  }
  return false;
}

inline void AArch64PartsEmulatedTimingPass::replacePACIA(MachineFunction &MF,
                                                 MachineBasicBlock &MBB,
                                                 MachineInstr &MI) {
  replacePAInstructionCommon(MF, MBB, MI);
  ++StatPacia;
}

inline void AArch64PartsEmulatedTimingPass::replaceAUTIA(MachineFunction &MF,
                                                 MachineBasicBlock &MBB,
                                                 MachineInstr &MI) {
  replacePAInstructionCommon(MF, MBB, MI);
  ++StatAutia;
}

void AArch64PartsEmulatedTimingPass::replaceBranchAuth(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineInstr &MI) {
  replacePAInstructionCommon(MF, MBB, MI);
  auto &MCI = getIndirectCallMachineInstruction(MI);
  BuildMI(MBB, MI, MI.getDebugLoc(), MCI).add(MI.getOperand(0));
  ++StatAuthBranch;
}

void AArch64PartsEmulatedTimingPass::replacePAInstructionCommon(MachineFunction &MF,
                                                    MachineBasicBlock &MBB,
                                                    MachineInstr &MI) {
  auto &mod = MI.getOperand(1);
  auto &dst = MI.getOperand(0);

  addNops(MBB, &MI, dst.getReg(), mod.getReg(), MI.getDebugLoc());
}

inline void AArch64PartsEmulatedTimingPass::addNops(MachineBasicBlock &MBB, MachineInstr *MI, unsigned ptrReg, unsigned modReg, const DebugLoc &DL) {
  BuildMI(MBB, MI, DL, TII->get(AArch64::EORXri), ptrReg).addReg(ptrReg).addImm(17);
  BuildMI(MBB, MI, DL, TII->get(AArch64::EORXri), ptrReg).addReg(ptrReg).addImm(37);
  BuildMI(MBB, MI, DL, TII->get(AArch64::EORXri), ptrReg).addReg(ptrReg).addImm(97);
  BuildMI(MBB, MI, DL, TII->get(AArch64::EORXrs), ptrReg).addReg(ptrReg).addReg(modReg).addImm(0);
}

inline const MCInstrDesc &AArch64PartsEmulatedTimingPass::getIndirectCallMachineInstruction(MachineInstr &MI) {

  if (MI.getOpcode() == AArch64::BLRAA)
      return TII->get(AArch64::BLR);

 return TII->get(AArch64::BR);
}
