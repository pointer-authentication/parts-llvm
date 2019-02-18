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
#include "llvm/CodeGen/RegisterScavenging.h"
// PARTS includes
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsCpiPass"

STATISTIC(StatAutia, DEBUG_TYPE ": code pointers authenticated and unPACed");
STATISTIC(StatPacia, DEBUG_TYPE ": code pointers signed");
STATISTIC(StatAutcall, DEBUG_TYPE ": inserted authenticate and branch instructions");

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsCpiPass : public MachineFunctionPass {

 public:
   static char ID;

   AArch64PartsCpiPass() : MachineFunctionPass(ID) {}

   StringRef getPassName() const override { return DEBUG_TYPE; }

   virtual bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 protected:
   virtual void doMachineFunctionInit(MachineFunction &MF);
   virtual void lowerPARTSAUTCALL(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   virtual void lowerPARTSPACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   virtual void replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dst, unsigned mod);

 private:
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;

   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline void lowerPARTSAUTIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   void lowerPARTSIntrinsicCommon(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI, const MCInstrDesc &InstrDesc);
   inline MachineInstr *findIndirectCallMachineInstr(MachineInstr *MI);
   inline void triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB);
   inline unsigned getFreeRegister(MachineBasicBlock &MBB, MachineInstr *MI_from, MachineInstr &MI_to);
   inline const MCInstrDesc &getIndirectCallMachineInstruction(MachineInstr *MI_incall);
   inline bool isPartsIntrinsic(unsigned Opcode);
   inline bool isIndirectCall(const MachineInstr &MI) const;
   inline void insertAuthenticateBranchInstr(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dstReg, unsigned modReg, const MCInstrDesc &InstrDesc);
   inline void insertMovInstr(MachineBasicBlock &MBB, MachineInstr *MI, unsigned dstReg, unsigned srcReg);
   inline bool isNormalIndirectCall(const MachineInstr *MI) const;

   friend class AArch64PartsCpiPassDecoratorBase;
 };

 class AArch64PartsCpiPassDecoratorBase : public AArch64PartsCpiPass {
  public:
    AArch64PartsCpiPassDecoratorBase(AArch64PartsCpiPass *CpiPass): AArch64PartsCpiPass() { PartsCpiPass = CpiPass; }
    virtual bool doInitialization(Module &M) override { return PartsCpiPass->doInitialization(M); };

  protected:
   PartsUtils_ptr  partsUtils = nullptr;

   void doMachineFunctionInit(MachineFunction &MF) override { PartsCpiPass->doMachineFunctionInit(MF); AArch64PartsCpiPass::doMachineFunctionInit(MF); partsUtils = PartsUtils::get(STI->getRegisterInfo(), TII); };
   virtual void lowerPARTSAUTCALL(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI) override { PartsCpiPass->lowerPARTSAUTCALL(MF, MBB, MI); };
   virtual void lowerPARTSPACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI) override { PartsCpiPass->lowerPARTSPACIA(MF, MBB, MI); };
   virtual void replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dst, unsigned mod) override { PartsCpiPass->replaceBranchByAuthenticatedBranch(MBB, MI_indcall, dst, mod); };

  private:
   AArch64PartsCpiPass *PartsCpiPass;
  };

 class AArch64PartsCpiWithRuntimeStatistics : public AArch64PartsCpiPassDecoratorBase {
  public:
    AArch64PartsCpiWithRuntimeStatistics(AArch64PartsCpiPass *PartsCpiPass) : AArch64PartsCpiPassDecoratorBase(PartsCpiPass) {}

    bool doInitialization(Module &M) override;

  private:
   Function *funcCountCodePtrBranch = nullptr;
   Function *funcCountCodePtrCreate = nullptr;

   void lowerPARTSAUTCALL(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI) override;
   void lowerPARTSPACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI) override;
 };

 class AArch64PartsCpiWithEmulatedTimings : public AArch64PartsCpiPassDecoratorBase {
  public:
    AArch64PartsCpiWithEmulatedTimings(AArch64PartsCpiPass *PartsCpiPass) : AArch64PartsCpiPassDecoratorBase(PartsCpiPass) {}

  private:
   virtual void replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dst, unsigned mod) override;
 };


} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsPassCpi() {
  AArch64PartsCpiPass *CpiPass = new AArch64PartsCpiPass();

  if (PARTS::useRuntimeStats())
    CpiPass = new AArch64PartsCpiWithRuntimeStatistics(CpiPass);

  if (PARTS::useDummy()) // True if we want to emulate auth instructions timings.
    CpiPass = new AArch64PartsCpiWithEmulatedTimings(CpiPass);

  return CpiPass;
}

char AArch64PartsCpiPass::ID = 0;

bool AArch64PartsCpiWithRuntimeStatistics::doInitialization(Module &M) {
  funcCountCodePtrBranch = PartsEventCount::getFuncCodePointerBranch(M);
  funcCountCodePtrCreate = PartsEventCount::getFuncCodePointerCreate(M);
  return AArch64PartsCpiPassDecoratorBase::doInitialization(M);
}

void AArch64PartsCpiWithRuntimeStatistics::lowerPARTSPACIA(MachineFunction &MF,
                                                 MachineBasicBlock &MBB,
                                                 MachineInstr &MI) {
  partsUtils->addEventCallFunction(MBB, MI, (--MachineBasicBlock::iterator(MI))->getDebugLoc(), funcCountCodePtrCreate);
  AArch64PartsCpiPassDecoratorBase::lowerPARTSPACIA(MF, MBB, MI);
}

void AArch64PartsCpiWithRuntimeStatistics::lowerPARTSAUTCALL(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineInstr &MI) {

  partsUtils->addEventCallFunction(MBB, *(--MachineBasicBlock::iterator(MI)), MI.getDebugLoc(), funcCountCodePtrBranch);
  AArch64PartsCpiPassDecoratorBase::lowerPARTSAUTCALL(MF, MBB, MI);
}

void AArch64PartsCpiWithEmulatedTimings::replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB,
                                                                                MachineInstr *MI_indcall,
                                                                                unsigned dst,
                                                                                unsigned mod) {
 // FIXME: This might break if the pointer is reused elsewhere!!!
 partsUtils->addNops(MBB, MI_indcall, dst, mod, MI_indcall->getDebugLoc());
}

bool AArch64PartsCpiPass::doInitialization(Module &M) {
  return true;
}

void AArch64PartsCpiPass::doMachineFunctionInit(MachineFunction &MF) {
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
}

bool AArch64PartsCpiPass::runOnMachineFunction(MachineFunction &MF) {
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
inline bool AArch64PartsCpiPass::handleInstruction(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();

  if (!isPartsIntrinsic(MIOpcode))
    return false;

  auto &MI = *MIi--;

  switch (MIOpcode) {
    default:
      llvm_unreachable("Unhandled PARTS intrinsic!!");
    case AArch64::PARTS_PACIA:
      lowerPARTSPACIA(MF, MBB, MI);
      break;
    case AArch64::PARTS_AUTIA:
      lowerPARTSAUTIA(MF, MBB, MI);
      break;
    case AArch64::PARTS_AUTCALL:
      lowerPARTSAUTCALL(MF, MBB, MI);
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
void AArch64PartsCpiPass::lowerPARTSPACIA(MachineFunction &MF,
                                                 MachineBasicBlock &MBB,
                                                 MachineInstr &MI) {
  lowerPARTSIntrinsicCommon(MF, MBB, MI, TII->get(AArch64::PACIA));
  ++StatPacia;
}

void AArch64PartsCpiPass::lowerPARTSAUTCALL(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineInstr &MI) {

  MachineInstr *MI_indcall = findIndirectCallMachineInstr(MI.getNextNode());
  if (MI_indcall == nullptr)
    triggerCompilationErrorOrphanAUTCALL(MBB);

  const unsigned mod_orig = MI.getOperand(2).getReg();
  const unsigned src = MI.getOperand(1).getReg();
  const unsigned dst = MI.getOperand(0).getReg();
  const unsigned mod = getFreeRegister(MBB, MI_indcall, MI);

  insertMovInstr(MBB, &MI, mod, mod_orig);
  if (src != dst)
    insertMovInstr(MBB, &MI, dst, src);

  replaceBranchByAuthenticatedBranch(MBB, MI_indcall, dst, mod);

  ++StatAutcall;
}

inline void AArch64PartsCpiPass::triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB) {
  DEBUG(MBB.dump());
  llvm_unreachable("failed to find BLR for AUTCALL");
}

void AArch64PartsCpiPass::replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB,
                                                                    MachineInstr *MI_indcall,
                                                                    unsigned dst,
                                                                    unsigned mod) {
  auto &MCI = getIndirectCallMachineInstruction(MI_indcall);
  insertAuthenticateBranchInstr(MBB, MI_indcall, dst, mod, MCI);
  MI_indcall->removeFromParent(); // Remove the replaced BR instruction
}

inline unsigned AArch64PartsCpiPass::getFreeRegister(MachineBasicBlock &MBB,
                                                     MachineInstr *MI_from,
                                                     MachineInstr &MI_to) {
  RegScavenger RS;
  auto &RC = AArch64::GPR64commonRegClass;

  RS.enterBasicBlockEnd(MBB);
  RS.backward(--MachineBasicBlock::iterator(MI_from));
  const unsigned reg = RS.scavengeRegisterBackwards(RC, MachineBasicBlock::iterator(MI_to), false, 0);
  RS.setRegUsed(reg); // Tell the Register Scavenger that the register is alive. Needed !?

  return reg;
}

inline const MCInstrDesc &AArch64PartsCpiPass::getIndirectCallMachineInstruction(MachineInstr *MI_indcall) {

  if (isNormalIndirectCall(MI_indcall))
   return TII->get(AArch64::BLRAA);

  // This is a tail call return, and we need to use BRAA
  // (tail-call: ~optimizatoin where a tail-cal is converted to a direct call so that
  // the tail-called function can return immediately to the current callee, without
  // going through the currently active function.)

 return TII->get(AArch64::BRAA);
}

inline void AArch64PartsCpiPass::lowerPARTSAUTIA(MachineFunction &MF,
                                                 MachineBasicBlock &MBB,
                                                 MachineInstr &MI) {
  lowerPARTSIntrinsicCommon(MF, MBB, MI, TII->get(AArch64::AUTIA));
  ++StatAutia;
}

void AArch64PartsCpiPass::lowerPARTSIntrinsicCommon(MachineFunction &MF,
                                                    MachineBasicBlock &MBB,
                                                    MachineInstr &MI,
                                                    const MCInstrDesc &InstrDesc) {
  const unsigned mod = MI.getOperand(2).getReg();
  const unsigned src = MI.getOperand(1).getReg();
  const unsigned dst = MI.getOperand(0).getReg();

  if (src != dst)
    insertMovInstr(MBB, &MI, dst, src);

  BuildMI(MBB, MI, MI.getDebugLoc(), InstrDesc)
    .addUse(dst)
    .addUse(mod);
}

inline MachineInstr *AArch64PartsCpiPass::findIndirectCallMachineInstr(MachineInstr *MI) {
  while (MI != nullptr && !isIndirectCall(*MI))
    MI = MI->getNextNode();

  return MI;
}

inline bool AArch64PartsCpiPass::isIndirectCall(const MachineInstr &MI) const {
  switch (MI.getOpcode()) {
    case AArch64::BLR:        // Normal indirect call
    case AArch64::TCRETURNri: // Indirect tail call
      return true;
  }
  return false;
}

inline void AArch64PartsCpiPass::insertAuthenticateBranchInstr(MachineBasicBlock &MBB,
                                                               MachineInstr *MI_indcall,
                                                               unsigned dstReg,
                                                               unsigned modReg,
                                                               const MCInstrDesc &InstrDesc) {
  auto BMI = BuildMI(MBB, MI_indcall, MI_indcall->getDebugLoc(), InstrDesc);
  BMI.addUse(dstReg);
  BMI.addUse(modReg);
}

inline void AArch64PartsCpiPass::insertMovInstr(MachineBasicBlock &MBB,
                                                MachineInstr *MI,
                                                unsigned dstReg,
                                                unsigned srcReg) {
  auto MOVMI = BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(AArch64::ORRXrs), dstReg);
  MOVMI.addUse(AArch64::XZR);
  MOVMI.addUse(srcReg);
  MOVMI.addImm(0);
}

inline bool AArch64PartsCpiPass::isNormalIndirectCall(const MachineInstr *MI) const {
  return MI->getOpcode() == AArch64::BLR;
}
