//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <iostream>
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
#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "AArch64RegisterInfo.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsEventCount.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsIntrPass"

using namespace llvm;
using namespace llvm::PARTS;

namespace {
class AArch64PartsIntrPass : public MachineFunctionPass {
public:
  static char ID;

  AArch64PartsIntrPass() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "parts-intrinsics"; }

  bool doInitialization(Module &M) override;
  bool runOnMachineFunction(MachineFunction &) override;

private:

  //const TargetMachine *TM = nullptr;
  const AArch64Subtarget *STI = nullptr;
  const AArch64InstrInfo *TII = nullptr;
  const AArch64RegisterInfo *TRI = nullptr;
  PartsUtils_ptr partsUtils;

  Function *funcCountDataStr = nullptr;
  Function *funcCountNonleafCall = nullptr;
  Function *funcCountLeafCall = nullptr;
};
} // end anonymous namespace

FunctionPass *llvm::createPartsPassIntrinsics() {
  return new AArch64PartsIntrPass();
}

char AArch64PartsIntrPass::ID = 0;

bool AArch64PartsIntrPass::doInitialization(Module &M) {
  funcCountDataStr = PartsEventCount::getFuncDataStr(M);
  funcCountNonleafCall = PartsEventCount::getFuncNonleafCall(M);
  funcCountLeafCall = PartsEventCount::getFuncLeafCall(M);
  return true;
}

bool AArch64PartsIntrPass::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;

  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  //TM = &MF.getTarget();;
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

  bool foundReturnSign = false;

  for (auto &MBB : MF) {
    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      const auto MIOpcode = MIi->getOpcode();

      switch(MIOpcode) {
        default:
          break;
        case AArch64::PACIB: {
          // FIXME: This return address signing counting should probably be properly put somewhere else...
          // Should however work as long as we only use PACIB for return address signing.
          if (PARTS::useRuntimeStats()) {
            const auto &DL = MIi->getDebugLoc();
            partsUtils->addEventCallFunction(MBB, *MIi, DL, funcCountNonleafCall);
            foundReturnSign = true;
            found = true;
          }
          break;
        }
        case AArch64::PARTS_PACDA:
        case AArch64::PARTS_AUTDA:
          break;
      }
    }
  }

  if (PARTS::useRuntimeStats()) {
    if (!foundReturnSign) {
      // if this function was instrumented we should already have found the PACIB earlier
      // Instrumenting the return instead, since instrumenting the entry prooved unreliable (guessing
      // empty functions, weird entry basic blocks, etc)
      for (auto &MBB : MF) {
        for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
          if (MIi->isReturn()) {
            partsUtils->addEventCallFunction(MBB, *MIi, MIi->getDebugLoc(), funcCountLeafCall);
            found = true;
          }
        }
      }
    }
  }

  return found;
}
