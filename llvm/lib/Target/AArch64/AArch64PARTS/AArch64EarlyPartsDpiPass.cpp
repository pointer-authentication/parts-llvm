//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// LLVM includes
#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "AArch64RegisterInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
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

#define DEBUG_TYPE "AArch64EarlyPartsDpiPass"

STATISTIC(StatUnneededDataPtr,
          DEBUG_TYPE " Number of uneeded data ptr intrisics removed");

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64EarlyPartsDpiPass : public MachineFunctionPass {

 public:
   static char ID;

   AArch64EarlyPartsDpiPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   virtual bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB,
                                      MachineBasicBlock::instr_iterator &MIi);
   inline bool removeIntrinsic( MachineBasicBlock::instr_iterator &MIi);
  };
} // end anonymous namespace

FunctionPass *llvm::createAArch64EarlyPartsPassDpi() {
  return new AArch64EarlyPartsDpiPass();
}

char AArch64EarlyPartsDpiPass::ID = 0;

bool AArch64EarlyPartsDpiPass::doInitialization(Module &M) {
  return true;
}

bool AArch64EarlyPartsDpiPass::runOnMachineFunction(MachineFunction &MF) {
  bool modified = false;

  for (auto &MBB : MF)
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie;
                                                                        ++MIi)
      modified = handleInstruction(MF, MBB, MIi) || modified;

  return modified;
}

inline bool AArch64EarlyPartsDpiPass::removeIntrinsic(
                                      MachineBasicBlock::instr_iterator &MIi) {
  auto &MI = *MIi--;

  MI.removeFromParent();
  StatUnneededDataPtr++;

  return true;
}

inline bool AArch64EarlyPartsDpiPass::handleInstruction(
                                       MachineFunction &MF,
                                       MachineBasicBlock &MBB,
                                       MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();

  if (MIOpcode != AArch64::PARTS_DATA_PTR)
    return false;

  auto srcReg = MIi->getOperand(0).getReg();
  auto &MRI = MF.getRegInfo();

  for (auto &DefI: MRI.def_instructions(srcReg))
    if (DefI.mayLoad())
      return removeIntrinsic(MIi);

  return false;
}
