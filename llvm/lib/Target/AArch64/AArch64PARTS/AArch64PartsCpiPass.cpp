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
#include "llvm/PARTS/PartsLog.h"
#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "AArch64PartsCpiPass"

STATISTIC(StatAutia, DEBUG_TYPE ": code pointers authenticated and unPACed");

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class AArch64PartsCpiPass : public MachineFunctionPass {

 public:
   static char ID;


   AArch64PartsCpiPass() :
   MachineFunctionPass(ID),
   log(PARTS::PartsLog::getLogger(DEBUG_TYPE))
   {
     DEBUG_PA(log->enable());
   }

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

 private:

   PartsLog_ptr log;

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;

   Function *funcCountCodePtrBranch = nullptr;
   Function *funcCountCodePtrCreate = nullptr;

   inline void initPartsOnMachineFunction(MachineFunction &MF);
   inline bool handleInstruction(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   inline void handlePartsIntrinsic(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi, unsigned MIOpcode);
   inline void LowerPARTSAUTCALL(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   inline void LowerPARTSAUTIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   inline void LowerPARTSPACIA(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);
   inline MachineInstr *FindIndirectCallMachineInstr(MachineInstr *MI);
   inline unsigned getFreeRegister(MachineBasicBlock &MBB, MachineInstr *MI_from, MachineInstr &MI_to);
   inline const MCInstrDesc &getIndirectCallMachineInstruction(MachineInstr *MI_incall);
   inline bool isPartsIntrinsic(unsigned Opcode);
   inline bool isIndirectCall(const MachineInstr &MI) const;
   inline void InsertAuthenticateBranchInstr(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dstReg, unsigned modReg, const MCInstrDesc &InstrDesc);
   inline void InsertMovInstr(MachineBasicBlock &MBB, MachineInstr *MI_autia, unsigned dstReg, unsigned srcReg);
   inline void ReplaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB, MachineInstr *MI_indcall, unsigned dst, unsigned mod);
   inline bool isNormalIndirectCall(const MachineInstr *MI) const;

 };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PartsPassCpi() {
  return new AArch64PartsCpiPass();
}

char AArch64PartsCpiPass::ID = 0;

bool AArch64PartsCpiPass::doInitialization(Module &M) {
  funcCountCodePtrBranch = PartsEventCount::getFuncCodePointerBranch(M);
  funcCountCodePtrCreate = PartsEventCount::getFuncCodePointerCreate(M);
  return true;
}

bool AArch64PartsCpiPass::runOnMachineFunction(MachineFunction &MF) {
  bool found = false;

  initPartsOnMachineFunction(MF);

  for (auto &MBB : MF) {
    DEBUG_PA(log->debug(MF.getName()) << "  block " << MBB.getName() << "\n");
    for (auto MIi = MBB.instr_begin(), MIie = MBB.instr_end(); MIi != MIie; ++MIi)
      found = handleInstruction(MF, MBB, MIi) || found;
  }

  return found;
}

inline void AArch64PartsCpiPass::initPartsOnMachineFunction(MachineFunction &MF)
{
  DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
  DEBUG_PA(log->debug() << "function " << MF.getName() << "\n");

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);
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

  DEBUG_PA(log->debug(MF.getName()) << "   " << MIi);

  if (!isPartsIntrinsic(MIOpcode))
    return false;

  handlePartsIntrinsic(MF, MBB, MIi, MIOpcode);

  return true;
}

inline void AArch64PartsCpiPass::handlePartsIntrinsic(MachineFunction &MF,
                                                       MachineBasicBlock &MBB,
                                                       MachineBasicBlock::instr_iterator &MIi,
                                                       unsigned MIOpcode) {
  auto &MI = *MIi--;

  if (MIOpcode == AArch64::PARTS_PACIA)
    LowerPARTSPACIA(MF, MBB, MI);
  else if (MIOpcode == AArch64::PARTS_AUTIA)
    LowerPARTSAUTIA(MF, MBB, MI);
  else
    LowerPARTSAUTCALL(MF, MBB, MI);
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
inline void AArch64PartsCpiPass::LowerPARTSPACIA( MachineFunction &MF,
                                                  MachineBasicBlock &MBB,
                                                  MachineInstr &MI) {

    log->inc(DEBUG_TYPE ".pacia", true) << "converting PARTS_PACIA\n";

    partsUtils->addEventCallFunction(MBB, MI, (--MachineBasicBlock::iterator(MI))->getDebugLoc(), funcCountCodePtrCreate);
    partsUtils->convertPartIntrinsic(MBB, MI, AArch64::PACIA);
}

inline void AArch64PartsCpiPass::LowerPARTSAUTCALL( MachineFunction &MF,
                                                  MachineBasicBlock &MBB,
                                                  MachineInstr &MI_autia) {
  log->inc(DEBUG_TYPE ".autia", true) << "converting PARTS_AUTCALL\n";

  const unsigned mod_orig = MI_autia.getOperand(2).getReg();
  const unsigned src = MI_autia.getOperand(1).getReg();
  const unsigned dst = MI_autia.getOperand(0).getReg();

  MachineInstr *MI_indcall = FindIndirectCallMachineInstr(MI_autia.getNextNode());
  if (MI_indcall == nullptr) {
      // This shouldn't happen, as it indicates that we didn't find what we were looking for
      // and have an orphaned pacia.
      DEBUG(MBB.dump()); // dump for debugging...
      llvm_unreachable("failed to find BLR for AUTCALL");
  }

  const unsigned mod = getFreeRegister(MBB, MI_indcall, MI_autia);
  InsertMovInstr(MBB, &MI_autia, mod, mod_orig);
  if (src != dst)
    InsertMovInstr(MBB, &MI_autia, dst, src);

  const auto DL = MI_indcall->getDebugLoc();
  partsUtils->addEventCallFunction(MBB, *(--MachineBasicBlock::iterator(MI_autia)), DL, funcCountCodePtrBranch);

  if (PARTS::useDummy())
    partsUtils->addNops(MBB, MI_indcall, src, mod, DL); // FIXME: This might break if the pointer is reused elsewhere!!!
  else
    ReplaceBranchByAuthenticatedBranch(MBB, MI_indcall, dst, mod);

  // Remove the PARTS intrinsic!
  MI_autia.removeFromParent();
}

inline void AArch64PartsCpiPass::ReplaceBranchByAuthenticatedBranch(MachineBasicBlock &MBB,
                                                                    MachineInstr *MI_indcall,
                                                                    unsigned dst,
                                                                    unsigned mod) {
    auto &MCI = getIndirectCallMachineInstruction(MI_indcall);
    InsertAuthenticateBranchInstr(MBB, MI_indcall, dst, mod, MCI);
    MI_indcall->removeFromParent(); // Remove the replaced BR instruction
}

inline unsigned AArch64PartsCpiPass::getFreeRegister( MachineBasicBlock &MBB,
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

inline void AArch64PartsCpiPass::LowerPARTSAUTIA( MachineFunction &MF,
                                                  MachineBasicBlock &MBB,
                                                  MachineInstr &MI_autia) {
  log->inc(DEBUG_TYPE ".autia", true) << "converting PARTS_AUTIA\n";

  const unsigned mod = MI_autia.getOperand(2).getReg();
  const unsigned dst = MI_autia.getOperand(0).getReg();

  BuildMI(MBB, MI_autia, MI_autia.getDebugLoc(), TII->get(AArch64::AUTIA))
    .addUse(dst)
    .addUse(mod);

  ++StatAutia;
  MI_autia.removeFromParent(); // Remove the PARTS intrinsic!
}


inline MachineInstr *AArch64PartsCpiPass::FindIndirectCallMachineInstr(MachineInstr *MI) {
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

inline void AArch64PartsCpiPass::InsertAuthenticateBranchInstr(MachineBasicBlock &MBB,
                                                                MachineInstr *MI_indcall,
                                                                unsigned dstReg,
                                                                unsigned modReg,
                                                                const MCInstrDesc &InstrDesc) {
  auto BMI = BuildMI(MBB, MI_indcall, MI_indcall->getDebugLoc(), InstrDesc);
  BMI.addUse(dstReg);
  BMI.addUse(modReg);
}

inline void AArch64PartsCpiPass::InsertMovInstr(MachineBasicBlock &MBB,
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
