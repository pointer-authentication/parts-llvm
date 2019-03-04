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
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "AArch64PartsPassCommon.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsDpiPass"

STATISTIC(StatDataStore, DEBUG_TYPE ": instrumented data stores");
STATISTIC(StatDataLoad, DEBUG_TYPE ": instrumented data loads");

using namespace llvm;
using namespace llvm::PARTS;

namespace {

class AArch64PartsDpiPass : public MachineFunctionPass, private AArch64PartsPassCommon {
public:
  static char ID;

  AArch64PartsDpiPass() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override { return DEBUG_TYPE; }
  bool runOnMachineFunction(MachineFunction &) override;

  bool lowerDpiIntrinsics(MachineFunction &MF);

private:
};

} // end anonymous namespace

FunctionPass *llvm::createPartsPassDpi() {
  return new AArch64PartsDpiPass();
}

char AArch64PartsDpiPass::ID = 0;

bool AArch64PartsDpiPass::runOnMachineFunction(MachineFunction &MF) {
  if (hasNoPartsAttribute(MF))
    return false;

  bool modified = false;

  initRunOn(MF);

  modified |= lowerDpiIntrinsics(MF);

  return modified;
}

bool AArch64PartsDpiPass::lowerDpiIntrinsics(MachineFunction& MF) {
  bool modified = false;

  for (auto &MBB : MF) {
    for (auto MBBI = MBB.begin(), end = MBB.end(); MBBI != end; ) {
      auto &MI = *MBBI++;

      switch(MI.getOpcode()) {
        default:
          break;
        case AArch64::PARTS_PACDA:
          replacePartsIntrinsic(MF, MBB, MI, TII->get(AArch64::PACDA));
          ++StatDataStore;
          modified = true;
          break;
        case AArch64::PARTS_AUTDA:
          replacePartsIntrinsic(MF, MBB, MI, TII->get(AArch64::AUTDA));
          modified = true;
          ++StatDataLoad;
          break;
      }
    }
  }

  return modified;
}

