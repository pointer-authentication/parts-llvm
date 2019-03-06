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

#define DEBUG_TYPE "AArch64PartsRuntimeStatistics"

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsRuntimeStatistics : public MachineFunctionPass {

 public:
   static char ID;

   AArch64PartsRuntimeStatistics() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   virtual bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
   PartsUtils_ptr  partsUtils = nullptr;
   Function *funcCountCodePtrBranch = nullptr;
   Function *funcCountCodePtrCreate = nullptr;
   inline bool handleInstruction(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline bool isPAInstr(unsigned Opcode);
  };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsPassRuntimeStatistics() {

  return new AArch64PartsRuntimeStatistics();
}

char AArch64PartsRuntimeStatistics::ID = 0;

bool AArch64PartsRuntimeStatistics::doInitialization(Module &M) {
  funcCountCodePtrBranch = PartsEventCount::getFuncCodePointerBranch(M);
  funcCountCodePtrCreate = PartsEventCount::getFuncCodePointerCreate(M);
  return true;
}

bool AArch64PartsRuntimeStatistics::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;
  const auto TRI = MF.getSubtarget<AArch64Subtarget>().getRegisterInfo();
  const auto TII = MF.getSubtarget<AArch64Subtarget>().getInstrInfo();

  partsUtils = PartsUtils::get(TRI, TII);

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
inline bool AArch64PartsRuntimeStatistics::handleInstruction(MachineBasicBlock &MBB,
                                                             MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();

  if (!isPAInstr(MIOpcode))
    return false;

  auto &MI = *MIi;

  switch (MIOpcode) {
    default:
      llvm_unreachable("Unhandled PAC instruction!!");
    case AArch64::PACIA:
      partsUtils->addEventCallFunction(MBB, MI, MI.getDebugLoc(), funcCountCodePtrCreate);
      break;
    case AArch64::BLRAA:
    case AArch64::TCRETURNriAA:
      partsUtils->addEventCallFunction(MBB, MI, MI.getDebugLoc(), funcCountCodePtrBranch);
      break;
  }

  return true;
}

inline bool AArch64PartsRuntimeStatistics::isPAInstr(unsigned Opcode) {
  switch (Opcode) {
//TODO:  case AArch64::AUTIA:
    case AArch64::PACIA:
    case AArch64::BLRAA:
    case AArch64::TCRETURNriAA:
      return true;
  }

  return false;
}
