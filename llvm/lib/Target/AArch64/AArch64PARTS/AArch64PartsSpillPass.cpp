//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea <carlos.chinea.perez@huawei.com>
//         Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
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
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
// PARTS includes
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "AArch64PartsPassCommon.h"

#define DEBUG_TYPE "AArch64PartsSpillPass"

STATISTIC(StatPartsSpills, DEBUG_TYPE ": Number of spills PACed");
STATISTIC(StatPartsReload, DEBUG_TYPE ": Number of PACed reloads");

using namespace llvm;
using namespace llvm::PARTS;

namespace {

  class AArch64PartsSpillPass : public MachineFunctionPass,
                                private AArch64PartsPassCommon {

  public:
    static char ID;

    AArch64PartsSpillPass() : MachineFunctionPass(ID) {}
    StringRef getPassName() const override { return DEBUG_TYPE; }

    bool doInitialization(Module &M) override;
    bool runOnMachineFunction(MachineFunction &) override;

  private:
    const AArch64InstrInfo *TII = nullptr;

    bool handleMBBInstructions(MachineBasicBlock &MBB);
    bool handleInstruction(MachineBasicBlock &MBB,
                           MachineBasicBlock::instr_iterator &MIi);
    void lowerPartsSpillIntrinsic(MachineBasicBlock &MBB,
                                  MachineInstr *InsertPoint,
                                  MachineInstr &MI,
                                  unsigned PACDesc,
                                  unsigned MemDesc);
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
  if (MF.getFunction().hasFnAttribute("no-parts")) return false;
  bool modified = false;

  TII = MF.getSubtarget<AArch64Subtarget>().getInstrInfo();

  for (auto &MBB : MF)
    modified = handleMBBInstructions(MBB);

  return modified;
}

bool AArch64PartsSpillPass::handleMBBInstructions(MachineBasicBlock &MBB) {
  bool modified = false;

  for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie; ++MIi)
    modified = handleInstruction(MBB, MIi) || modified;

  return modified;
}

bool AArch64PartsSpillPass::handleInstruction(MachineBasicBlock &MBB,
                                      MachineBasicBlock::instr_iterator &MIi) {

  auto &MI = *MIi;

  switch(MI.getOpcode()) {
    case AArch64::PARTS_SPILL:
      lowerPartsSpillIntrinsic(MBB, &MI, MI, AArch64::PARTS_PACDA, AArch64::STRXui);
      StatPartsSpills++;
      break;
    case AArch64::PARTS_USPILL:
      lowerPartsSpillIntrinsic(MBB, &MI, MI, AArch64::PARTS_PACDA, AArch64::STURXi);
      StatPartsSpills++;
      break;
   case AArch64::PARTS_RELOAD:
      lowerPartsSpillIntrinsic(MBB, MI.getNextNode(), MI, AArch64::PARTS_AUTDA,
                                                              AArch64::LDRXui);
      StatPartsReload++;
      break;
   case AArch64::PARTS_URELOAD:
      lowerPartsSpillIntrinsic(MBB, MI.getNextNode(), MI, AArch64::PARTS_AUTDA,
                                                              AArch64::LDURXi);
      StatPartsReload++;
      break;
   case AArch64::PARTS_DATA_PTR:
      --MIi;
      MI.removeFromParent();
      break;
    default:
      return false;
      break;
  }

  return true;
}

void AArch64PartsSpillPass::lowerPartsSpillIntrinsic(MachineBasicBlock &MBB,
                                                     MachineInstr *InsertPoint,
                                                     MachineInstr &MI,
                                                     unsigned PACDesc,
                                                     unsigned MemDesc) {
  unsigned Reg = MI.getOperand(0).getReg();

  if (InsertPoint)
    BuildMI(MBB, InsertPoint, MI.getDebugLoc(), TII->get(PACDesc), Reg)
        .addUse(Reg)
        .addUse(AArch64::SP);
  else
    BuildMI(&MBB, MI.getDebugLoc(), TII->get(PACDesc), Reg)
        .addUse(Reg)
        .addUse(AArch64::SP);

  MI.setDesc(TII->get(MemDesc));
}
