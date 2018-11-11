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
#include "llvm/PARTS/PartsEventCount.h"
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

   Function *funcCountCodePtrBranch = nullptr;
 };
} // end anonymous namespace

FunctionPass *llvm::createPartsPassCpi() {
  return new PartsPassCpi();
}

char PartsPassCpi::ID = 0;

bool PartsPassCpi::doInitialization(Module &M) {
  funcCountCodePtrBranch = PartsEventCount::getFuncCodePointerBranch(M);
  return true;
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

      if (MIi->isCall() || MIi->isIndirectBranch()) {
        /* We don't expect to be instrumenting all branches, but lets just keep them here
         * to make sure we are aware of anything that might need to be instrumented. */
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
  const auto MIName = TII->getName(MIOpcode).str();
  auto partsType = PartsTypeMetadata::retrieve(*MIi);

  skipIfN(MIOpcode == AArch64::BL ||
          //MIOpcode == AArch64::Bcc ||
          //MIOpcode == AArch64::TBZW ||
          //MIOpcode == AArch64::TBZX ||
          MIi->isReturn(),
          "Branch.Skipped_" + MIName, "skipping branch '" + MIName + "'!\n");

  /* Assume that only these are left... */
  assert(MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL || MIOpcode == AArch64::BR);

  /* ----------------------------- BL/BLR ---------------------------------------- */
  DEBUG_PA(log->debug(MF.getName()) << "      found a BL/BLR (" << TII->getName(MIOpcode) << ")\n");

  if (partsType == nullptr) {
    DEBUG_PA(log->debug(MF.getName()) << "      trying to figure out type_id\n");

    assert(MIi->getNumOperands() >= 1);
    assert(MIi->getOperand(0).isReg());

    const auto reg = MIi->getOperand(0).getReg();

    // Only look within same basic block
    if (MBB.instr_begin() != MIi) {
      auto iter = MIi;

      do {
        iter--;

        for (unsigned i = 0; i < iter->getNumOperands(); i++) {
          auto foundOp = MIi->getOperand(i);
          if (foundOp.isReg() && foundOp.getReg() == reg) {

            if (AArch64::LDRXroX == iter->getOpcode()) {
              // FIXME: is this actually true!??
              // This is probably a jump-table, either way, it is marked RO...
              partsType = PartsTypeMetadata::getIgnored();
            }

            // Just a stupid way to exit the do loop...
            i = UINT_MAX/2;
            iter = MBB.instr_begin();
          }
        }
      } while (MBB.instr_begin() != iter);
    }


    if (partsType == nullptr)
      partsType = PartsTypeMetadata::getUnknown();
  }

  skipIfN(partsType->isIgnored(), "Branch.Ignored_" + MIName, "marked as ignored, skipping!\n");
  skipIfB(!partsType->isKnown(), "Branch.Unknown_" + MIName, false, "type_id is unknown!\n");
  skipIfN(!partsType->isPointer(), "Branch.NotAPointer_" + MIName, "not a pointer, skipping!\n");

  assert(MIOpcode != AArch64::BL && "Whoops, thought this was never, maybe, gonna happen. I guess?");

  log->inc("Branch.Instrumented_" + MIName,  true) << "instrumenting call " << partsType->toString() << "\n";

  const auto ptrRegOperand = MIi->getOperand(0);
  const auto DL = MIi->getDebugLoc();
  const auto modReg = PARTS::getModifierReg();


  partsUtils->addEventCallFunction(MBB, *MIi, DL, funcCountCodePtrBranch);

  // Create the PAC modifier
  partsUtils->moveTypeIdToReg(MBB, MIi, modReg, partsType->getTypeId(), DL);

  // Swap out the branch to a auth+branch variant
  if (!PARTS::useDummy()) {
    auto BMI = BuildMI(MBB, *MIi, DL, TII->get(AArch64::BLRAA));
    BMI.add(ptrRegOperand);
    BMI.addReg(modReg);

    // Remove the old instruction!
    auto &MI = *MIi;
    MIi--;
    MI.removeFromParent();
  } else {
    const auto ptrReg = ptrRegOperand.getReg();
    partsUtils->addNops(MBB, MIi == MBB.instr_end() ? nullptr : &*MIi, ptrReg, modReg, DL);
  }

  return true;
}
