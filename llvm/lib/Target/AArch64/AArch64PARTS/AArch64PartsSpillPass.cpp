//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea <carlos.chinea.perez@huawei.com>
// Copyright (C) 2019 Huawei Finland Oy
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
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsSpillPass"

STATISTIC(StatPartsSpills, DEBUG_TYPE ": Number of spills PACed");

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsSpillPass : public MachineFunctionPass {

 public:
   static char ID;

   AArch64PartsSpillPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:
   const AArch64InstrInfo *TII = nullptr;

 };

} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsSpillPass() {
  return new AArch64PartsSpillPass();
}

char AArch64PartsSpillPass::ID = 0;

bool AArch64PartsSpillPass::doInitialization(Module &M) {
  return true;
}

bool AArch64PartsSpillPass::runOnMachineFunction(MachineFunction &MF) {
  bool modified = false;

  TII = MF.getSubtarget<AArch64Subtarget>().getInstrInfo();

#if 0
  for (auto &MBB : MF)
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie; ++MIi)
      modified = handleInstruction(MBB, MIi) || modified;
#endif
  StatPartsSpills = 0;

  return modified;
}
