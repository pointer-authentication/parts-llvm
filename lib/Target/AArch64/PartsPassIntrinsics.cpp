/*
 * PartsPassIntrinsics.cpp
 * Copyright (C) 2018 Secure Systems Group, Aalto University, ssg.aalto.fi
 * Author: Hans Liljestrand <liljestrandh@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */


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
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "PointerAuthentication.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "aarch64-pa-forwardcfi"

using namespace llvm;
using namespace llvm::PA;

namespace {
class PartsPassIntrinsics : public MachineFunctionPass {
public:
  static char ID;

  PartsPassIntrinsics() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "parts-intrinsics"; }

  bool doInitialization(Module &M) override;
  bool runOnMachineFunction(MachineFunction &) override;

private:

  //const TargetMachine *TM = nullptr;
  const AArch64Subtarget *STI = nullptr;
  const AArch64InstrInfo *TII = nullptr;
  const AArch64RegisterInfo *TRI = nullptr;
  PartsUtils_ptr partsUtils;
};
} // end anonymous namespace

FunctionPass *llvm::createPartsPassIntrinsics() {
  return new PartsPassIntrinsics();
}

char PartsPassIntrinsics::ID = 0;

bool PartsPassIntrinsics::doInitialization(Module &M) {
  return false;
}

bool PartsPassIntrinsics::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;

  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  //TM = &MF.getTarget();;
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

  for (auto &MBB : MF) {
    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      const auto MIOpcode = MIi->getOpcode();

      if (MIOpcode == AArch64::PARTS_PACIA) {

        // Find our registers and the PA modifier
        unsigned SrcReg = MIi->getOperand(1).getReg();
        unsigned DstReg = MIi->getOperand(0).getReg();
        unsigned ModReg = Pauth_ModifierReg;
        auto type_id = static_cast<type_id_t>(MIi->getOperand(2).getImm());

        // PAC the pointer
        partsUtils->pacCodePointer(MBB, MIi, DstReg, SrcReg, ModReg, type_id);

        // And finally, remove the intrinsic
        auto tmp = MIi;
        MIi--;
        tmp->removeFromParent();

        found = true; // make sure we return true when we modify stuff
      }
    }
  }

  return found;
}
