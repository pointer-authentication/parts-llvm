//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea <carlos.chinea.perez@huawei.com>
// Copyright (C) 2019 Huawei Finland Oy
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
#include "AArch64PartsPassCommon.h"

#define DEBUG_TYPE "AArch64PartsSpillPass"

STATISTIC(StatPartsSpills, DEBUG_TYPE ": Number of spills PACed");
STATISTIC(StatPartsReload, DEBUG_TYPE ": Number of PACed reloads");

using namespace llvm;
using namespace llvm::PARTS;

namespace {
  #define RM_INSTR_SIZE 32  //TODO: Fine tune this value. Currently chosen by a hunch :-P

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
    SmallVector<MachineInstr *, RM_INSTR_SIZE> DeleteInstrList;

    bool handleInstruction(MachineBasicBlock &MBB,
                           MachineBasicBlock::instr_iterator &MIi);
    void lowerPartsSpillIntrinsic(MachineBasicBlock &MBB,
                                  MachineInstr *InsertPoint,
                                  MachineInstr &MI,
                                  unsigned PACDesc,
                                  unsigned MemDesc);
    void removeIntrinsic(MachineBasicBlock &MBB,
                         MachineBasicBlock::instr_iterator &MIi);
    void removeDefsWithNoUses(MachineRegisterInfo &MRI, unsigned srcReg);
  };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsSpillPass() {
  return new AArch64PartsSpillPass();
}

char AArch64PartsSpillPass::ID = 0;

bool AArch64PartsSpillPass::doInitialization(Module &M) {
  return true;
}

void AArch64PartsSpillPass::lowerPartsSpillIntrinsic(MachineBasicBlock &MBB,
                                                     MachineInstr *InsertPoint,
                                                     MachineInstr &MI,
                                                     unsigned PACDesc,
                                                     unsigned MemDesc)
{
  auto &MO = MI.getOperand(0);

  if (InsertPoint)
    BuildMI(MBB, InsertPoint, MI.getDebugLoc(), TII->get(PACDesc), MO.getReg())
        .addUse(AArch64::SP);
  else
    BuildMI(&MBB, MI.getDebugLoc(), TII->get(PACDesc), MO.getReg())
        .addUse(AArch64::SP);

  MI.setDesc(TII->get(MemDesc));
  MO.setIsRenamable(false);
}

bool AArch64PartsSpillPass::handleInstruction(MachineBasicBlock &MBB,
                                      MachineBasicBlock::instr_iterator &MIi) {

  auto &MI = *MIi;

  switch(MI.getOpcode()) {
    case AArch64::PARTS_SPILL:
      lowerPartsSpillIntrinsic(MBB, &MI, MI, AArch64::PACDA, AArch64::STRXui);
      StatPartsSpills++;
      break;
    case AArch64::PARTS_USPILL:
      lowerPartsSpillIntrinsic(MBB, &MI, MI, AArch64::PACDA, AArch64::STURXi);
      StatPartsSpills++;
      break;
   case AArch64::PARTS_RELOAD:
      lowerPartsSpillIntrinsic(MBB, MI.getNextNode(), MI, AArch64::AUTDA,
                                                              AArch64::LDRXui);
      StatPartsReload++;
      break;
   case AArch64::PARTS_URELOAD:
      lowerPartsSpillIntrinsic(MBB, MI.getNextNode(), MI, AArch64::AUTDA,
                                                              AArch64::LDURXi);
      StatPartsReload++;
      break;
   case AArch64::PARTS_DATA_PTR:
      removeIntrinsic(MBB, MIi);
      break;
    default:
      return false;
      break;
  }

  return true;
}

#if 0
static void removeDefsWithNoUses(MachineRegisterInfo &MRI, unsigned srcReg)
{
  for (auto Ii = MRI.def_instr_begin(srcReg), Ei = MRI.def_instr_end(),
      NextIi = Ii; Ii != Ei; Ii = NextIi) {
    NextIi = std::next(Ii);
    Ii->removeFromParent();
  }

}
#else
void AArch64PartsSpillPass::removeDefsWithNoUses(MachineRegisterInfo &MRI, unsigned srcReg)
{
  for (auto Ii = MRI.def_instr_begin(srcReg), Ei = MRI.def_instr_end(),
      NextIi = Ii; Ii != Ei; Ii = NextIi) {
    NextIi = std::next(Ii);
    DeleteInstrList.push_back(&*Ii);
  }
}

#endif

void AArch64PartsSpillPass::removeIntrinsic(MachineBasicBlock &MBB,
                                      MachineBasicBlock::instr_iterator &MIi) {
  auto &MI = *MIi--;
  auto &MRI = MBB.getParent()->getRegInfo();
  auto &MO = MI.getOperand(0);
  unsigned srcReg = MO.getReg();

  MI.removeFromParent();

  if (MRI.use_empty(srcReg))
    removeDefsWithNoUses(MRI, srcReg); // FIXME: Do not remove in place, may turn MIi invalid. Accumulate in a worklist the registers and remove at the end of the basic block processing
}

bool AArch64PartsSpillPass::runOnMachineFunction(MachineFunction &MF) {
  bool modified = false;

  TII = MF.getSubtarget<AArch64Subtarget>().getInstrInfo();

  for (auto &MBB : MF) {
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie;
                                                                         ++MIi)
      modified = handleInstruction(MBB, MIi) || modified;

    for(auto MI: DeleteInstrList) {
      errs() << "REMOVING MI: " << *MI;
      MI->removeFromParent();
    }
    DeleteInstrList.clear();
  }
  return modified;
}
