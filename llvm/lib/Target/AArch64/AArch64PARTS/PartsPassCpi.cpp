//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
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
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/PARTS/PartsLog.h"
#include "llvm/PARTS/Parts.h"
#include "PartsUtils.h"

#define DEBUG_TYPE "aarch64-parts-cpi"

using namespace llvm;
using namespace llvm::PARTS;

#define skipIfB(ifx, stat, b, string) do {  \
    if ((ifx)) {                            \
      log->inc(stat, b) << string;          \
      return false;                         \
    }                                       \
} while(false)

#define skipIfN(ifx, stat, string) do {     \
    if ((ifx)) {                            \
      log->inc(stat) << string;             \
      return false;                         \
    }                                       \
} while (false)

namespace {
 class PartsPassCpi : public MachineFunctionPass {

 public:
   static char ID;

   PartsPassCpi() :
   MachineFunctionPass(ID),
   log(PARTS::PartsLog::getLogger(DEBUG_TYPE))
   {
     DEBUG_PA(log->enable());
   }

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;
   bool instrumentBranches(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);

 private:

   PartsLog_ptr log;

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;
 };
} // end anonymous namespace

FunctionPass *llvm::createPartsPassCpi() {
  return new PartsPassCpi();
}

char PartsPassCpi::ID = 0;

bool PartsPassCpi::doInitialization(Module &M) {
  return false;
}

bool PartsPassCpi::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
  DEBUG_PA(log->debug() << "function " << MF.getName() << "\n");

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

  for (auto &MBB : MF) {
    DEBUG_PA(log->debug(MF.getName()) << "  block " << MBB.getName() << "\n");

    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      DEBUG_PA(log->debug(MF.getName()) << "   " << MIi);

      const auto MIOpcode = MIi->getOpcode();

      if (MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL) {
        instrumentBranches(MF, MBB, MIi);
      }
    }
  }

  return true;
}

bool PartsPassCpi::instrumentBranches(MachineFunction &MF,
                                      MachineBasicBlock &MBB,
                                      MachineBasicBlock::instr_iterator &MIi) {
  if (!PARTS::useFeCfi())
    return false;

  const auto MIOpcode = MIi->getOpcode();
  auto partsType = PartsTypeMetadata::retrieve(*MIi);

  assert(MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL);

  /* ----------------------------- BL/BLR ---------------------------------------- */
  DEBUG_PA(log->debug(MF.getName()) << "      found a BL/BLR (" << TII->getName(MIOpcode) << ")\n");

  if (partsType == nullptr) {
    DEBUG_PA(log->debug(MF.getName()) << "      trying to figure out type_id\n");
    log->error(MF.getName()) << __FUNCTION__ << ": figuring out NOT IMPLEMENTED!!!\n";

    partsType = PartsTypeMetadata::getUnknown();
  }

  skipIfB(!partsType->isKnown(), "Branch.Unknown", false, "type_id is unknown!\n");
  skipIfN(partsType->isIgnored(), "Branch.Ignored", "marked as ignored, skipping!\n");
  skipIfN(!partsType->isPointer(), "Branch.NotAPointer", "not a pointer, skipping!\n");

  log->inc("StoreLoad.InstrumentedCall", true) << "      instrumenting call " << partsType->toString() << "\n";
  assert(MIOpcode != AArch64::BL && "Whoops, thought this was never, maybe, gonna happen. I guess?");

  const auto ptrRegOperand = MIi->getOperand(0);
  const auto DL = MIi->getDebugLoc();
  const auto modReg = PARTS::getModifierReg();

  // Create the PAC modifier
  partsUtils->moveTypeIdToReg(MBB, MIi, modReg, partsType->getTypeId(), DL);

  // Swap out the branch to a auth+branch variant
#ifndef USE_DUMMY_INSTRUCTIONS
  auto BMI = BuildMI(MBB, *MIi, DL, TII->get(AArch64::BLRAA));
  BMI.add(ptrRegOperand);
  BMI.addReg(modReg);

  // Remove the old instruction!
  auto &MI = *MIi;
  MIi--;
  MI.removeFromParent();
#else
  const auto ptrReg = ptrRegOperand.getReg();
  MIi->dump();
  partsUtils->addNops(MBB, MIi == MBB.instr_end() ? nullptr : &*MIi, ptrReg, modReg, DL);
#endif // USE_DUMMY_INSTRUCTIONS

  return true;
}
