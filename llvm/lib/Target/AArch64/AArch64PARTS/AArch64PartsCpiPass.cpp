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
   PartsLog_ptr log;

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;

   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline void lowerPARTSAUTIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   virtual void lowerPARTSAUTCALL(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   virtual void lowerPARTSPACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   void lowerPARTSIntrinsicCommon(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI, const MCInstrDesc &InstrDesc);
   inline MachineInstr *findIndirectCallMachineInstr(MachineInstr *MI);
   inline void triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB);
   inline unsigned getFreeRegister(MachineBasicBlock &MBB, MachineInstr *MI_from, MachineInstr &MI_to);
   inline const MCInstrDesc &getIndirectCallMachineInstruction(MachineInstr *MI_incall);
   inline bool isPartsIntrinsic(unsigned Opcode);
   inline bool isIndirectCall(const MachineInstr &MI) const;
   inline void insertAuthenticateBranchInstr(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dstReg, unsigned modReg, const MCInstrDesc &InstrDesc);
   inline void insertMovInstr(MachineBasicBlock &MBB, MachineInstr *MI_autia, unsigned dstReg, unsigned srcReg);
   inline void replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dst, unsigned mod);
   inline bool isNormalIndirectCall(const MachineInstr *MI) const;

 };

 class AArch64PartsCpiWithEventCallPass : public AArch64PartsCpiPass {
  public:
    AArch64PartsCpiWithEventCallPass() : AArch64PartsCpiPass() {}

    bool doInitialization(Module &M) override;

  private:
   Function *funcCountCodePtrBranch = nullptr;
   Function *funcCountCodePtrCreate = nullptr;

   void lowerPARTSAUTCALL(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI) override;
   void lowerPARTSPACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI) override;
 };

} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsPassCpi() {
  if (PARTS::useRuntimeStats())
   return new AArch64PartsCpiWithEventCallPass();
  else
   return new AArch64PartsCpiPass();
}

char AArch64PartsCpiPass::ID = 0;

bool AArch64PartsCpiWithEventCallPass::doInitialization(Module &M) {
  funcCountCodePtrBranch = PartsEventCount::getFuncCodePointerBranch(M);
  funcCountCodePtrCreate = PartsEventCount::getFuncCodePointerCreate(M);
  return true;
}

void AArch64PartsCpiWithEventCallPass::lowerPARTSPACIA(MachineFunction &MF,
                                                 MachineBasicBlock &MBB,
                                                 MachineInstr &MI) {
  partsUtils->addEventCallFunction(MBB, MI, (--MachineBasicBlock::iterator(MI))->getDebugLoc(), funcCountCodePtrCreate);
  AArch64PartsCpiPass::lowerPARTSPACIA(MF, MBB, MI);
}

void AArch64PartsCpiWithEventCallPass::lowerPARTSAUTCALL(MachineFunction &MF,
                                                   MachineBasicBlock &MBB,
                                                   MachineInstr &MI_autia) {

  partsUtils->addEventCallFunction(MBB, *(--MachineBasicBlock::iterator(MI_autia)), MI_autia.getDebugLoc(), funcCountCodePtrBranch);
  AArch64PartsCpiPass::lowerPARTSAUTCALL(MF, MBB, MI_autia);
}

bool AArch64PartsCpiPass::doInitialization(Module &M) {
  return true;
}

bool AArch64PartsCpiPass::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

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
                                                   MachineInstr &MI_autia) {

  MachineInstr *MI_indcall = findIndirectCallMachineInstr(MI_autia.getNextNode());
  if (MI_indcall == nullptr)
    triggerCompilationErrorOrphanAUTCALL(MBB);

  const unsigned mod_orig = MI_autia.getOperand(2).getReg();
  const unsigned src = MI_autia.getOperand(1).getReg();
  const unsigned dst = MI_autia.getOperand(0).getReg();
  const unsigned mod = getFreeRegister(MBB, MI_indcall, MI_autia);

  insertMovInstr(MBB, &MI_autia, mod, mod_orig);
  if (src != dst)
    insertMovInstr(MBB, &MI_autia, dst, src);

  if (PARTS::useDummy())  // True if we want to emulate auth instructions timings.
    partsUtils->addNops(MBB, MI_indcall, src, mod, MI_indcall->getDebugLoc()); // FIXME: This might break if the pointer is reused elsewhere!!!
  else
    replaceBranchByAuthenticatedBranch(MBB, MI_indcall, dst, mod);

  ++StatAutcall;
}

inline void AArch64PartsCpiPass::triggerCompilationErrorOrphanAUTCALL(MachineBasicBlock &MBB) {
  DEBUG(MBB.dump());
  llvm_unreachable("failed to find BLR for AUTCALL");
}

inline void AArch64PartsCpiPass::replaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB,
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
                                                MachineInstr *MI_autia,
                                                unsigned dstReg,
                                                unsigned srcReg) {
  auto MOVMI = BuildMI(MBB, MI_autia, MI_autia->getDebugLoc(), TII->get(AArch64::ORRXrs), dstReg);
  MOVMI.addUse(AArch64::XZR);
  MOVMI.addUse(srcReg);
  MOVMI.addImm(0);
}

inline bool AArch64PartsCpiPass::isNormalIndirectCall(const MachineInstr *MI) const {
  return MI->getOpcode() == AArch64::BLR;
}
