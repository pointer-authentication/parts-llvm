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
#include "PartsUtils.h"

#define DEBUG_TYPE "aarch64-pa-forwardcfi"

using namespace llvm;
using namespace llvm::PARTS;

namespace {
 class PartsPassPointerLoadStore : public MachineFunctionPass {

 public:
   static char ID;

   PartsPassPointerLoadStore() :
   MachineFunctionPass(ID),
   log(PARTS::PartsLog::getLogger(DEBUG_TYPE))
   {
     DEBUG_PA(log->enable());
   }

   StringRef getPassName() const override { return "pauth-sllow"; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;

   bool instrumentBranches(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
   bool instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, type_id_t type_id);
   bool instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI, unsigned pointerReg, type_id_t type_id);

 private:

   PartsLog_ptr log;

   const TargetMachine *TM = nullptr;
   const AArch64Subtarget *STI = nullptr;
   const AArch64InstrInfo *TII = nullptr;
   const AArch64RegisterInfo *TRI = nullptr;
   PartsUtils_ptr  partsUtils = nullptr;
 };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PaForwardCfiPass() {
  return new PartsPassPointerLoadStore();
}

char PartsPassPointerLoadStore::ID = 0;

bool PartsPassPointerLoadStore::doInitialization(Module &M) {
  return false;
}

bool PartsPassPointerLoadStore::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
  DEBUG_PA(log->debug() << "function " << MF.getName() << "\n");

  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
  partsUtils = PartsUtils::get(TRI, TII);

  auto &C = MF.getFunction().getContext();
  DEBUG_PA(const auto fName = MF.getName());

  for (auto &MBB : MF) {
    DEBUG_PA(log->debug(fName) << "  block " << MBB.getName() << "\n");

    for (auto MIi = MBB.instr_begin(); MIi != MBB.instr_end(); MIi++) {
      DEBUG_PA(log->debug(fName) << "   " << MIi);

      const auto MIOpcode = MIi->getOpcode();
      auto partsType = PartsTypeMetadata::retrieve(*MIi);

      if (MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL) {
        instrumentBranches(MBB, MIi);
      } else if (partsUtils->isLoadOrStore(*MIi)) {
        /* ----------------------------- LOAD/STORE ---------------------------------------- */
        DEBUG_PA(log->debug(fName) << "      found a load/store (" << TII->getName(MIOpcode) << ")\n");

        if (partsType == nullptr) {
          DEBUG_PA(log->debug(fName) << "      trying to figure out type_id\n");
          auto Op = MIi->getOperand(0);
          const auto targetReg = Op.getReg();

          if (!partsUtils->checkIfRegInstrumentable(targetReg)) {
            partsType = PartsTypeMetadata::getIgnored();
          } else {
            if (partsUtils->isStore(*MIi)) {
              partsType = partsUtils->inferPauthTypeIdRegBackwards(MF, MBB, *MIi, targetReg);
            } else {
              // FIXME: this only supports loads of type load reg [reg, imm]
              if (MIi->getOperand(2).isImm()) {
                partsType = partsUtils->inferPauthTypeIdStackBackwards(MF, MBB, *MIi, targetReg, MIi->getOperand(1).getReg(), MIi->getOperand(2).getImm());
              } else {
                log->error() << __FUNCTION__ << ": OMG! unexpected operands, is this a pair store thingy?\n";
                partsType = PartsTypeMetadata::getUnknown();
              }
            }
          }
          partsUtils->attach(MF.getFunction().getContext(), partsType, &*MIi);

          MIi->addOperand(MachineOperand::CreateMetadata(partsType->getMDNode(C)));
          log->inc("StoreLoad.Inferred") << "      storing type_id " << partsType->toString() << ") in current MI\n";
        }

        if (!partsType->isKnown()) {
          log->inc("StoreLoad.Unknown", false) << "      type_id is unknown!\n";
          continue;
        }

        if (partsType->isIgnored()) {
          log->inc("StoreLoad.Ignored") << "      marked as ignored, skipping\n";
          continue;
        }

        if (!partsType->isPointer()) {
          log->inc("StoreLoad.NotAPointer") << "      not a pointer, skipping\n";
          continue;
        }

        if (partsType->isDataPointer()) {
          log->inc("StoreLoad.InstrumentedData", true) << "      instrumenting store/load " << partsType->toString() << "\n";
        } else {
          assert(partsType->isCodePointer());
          log->inc("StoreLoad.InstrumentedCode", true) << "      instrumenting store/load " << partsType->toString() << "\n";
        }

        auto reg = MIi->getOperand(0).getReg();
        const auto modReg = PARTS::getModifierReg();
        const auto type_id = partsType->getTypeId();

        if (partsUtils->isStore(*MIi)) {
          if (partsType->isDataPointer())
            partsUtils->pacDataPointer(MBB, MIi, reg, modReg, type_id, MIi->getDebugLoc());
          else
            partsUtils->pacCodePointer(MBB, MIi, reg, modReg, type_id, MIi->getDebugLoc());
        } else {
          auto loc = MIi;
          MIi->getDebugLoc();
          loc++;
          if (partsType->isDataPointer())
            partsUtils->autDataPointer(MBB, loc, reg, modReg, type_id, MIi->getDebugLoc());
          else
            partsUtils->autCodePointer(MBB, loc, reg, modReg, type_id, MIi->getDebugLoc());
        }
      }
    }
  }

  return true;
}

bool PartsPassPointerLoadStore::instrumentBranches(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi) {
  const auto MIOpcode = MIi->getOpcode();
  const auto fName = MBB.getParent()->getName();
  auto partsType = PartsTypeMetadata::retrieve(*MIi);

  assert(MIOpcode == AArch64::BLR || MIOpcode == AArch64::BL);

  /* ----------------------------- BL/BLR ---------------------------------------- */
  DEBUG_PA(log->debug(fName) << "      found a BL/BLR (" << TII->getName(MIOpcode) << ")\n");

  if (partsType == nullptr) {
    DEBUG_PA(log->debug(fName) << "      trying to figure out type_id\n");
    log->error(fName) << __FUNCTION__ << ": figuring out NOT IMPLEMENTED!!!\n";

    partsType = PartsTypeMetadata::getUnknown();
  }

  if (!partsType->isKnown()) {
    log->inc("Branch.Unknown", false) << "      type_id is unknown!\n";
    return false;
  }

  if (partsType->isIgnored()) {
    log->inc("Branch.Ignored") << "      marked as ignored, skipping\n";
    return false;
  }

  if (!partsType->isPointer()) {
    log->inc("Branch.NotAPointer") << "      not a pointer, skipping\n";
    return false;
  }

  log->inc("StoreLoad.InstrumentedCall", true) << "      instrumenting call " << partsType->toString() << "\n";
  assert(MIOpcode != AArch64::BL && "Whoops, thought this was never, maybe, gonna happen. I guess?");

  // Create the PAC modifier
  partsUtils->moveTypeIdToReg(MBB, MIi, PARTS::getModifierReg(), partsType->getTypeId(), MIi->getDebugLoc());
  /*
  BuildMI(MBB, *MIi, DebugLoc(), TII->get(AArch64::MOVZXi))
      .addReg(PARTS::getModifierReg())
      .addImm(partsType->getTypeId())
      .addImm(0);
  */

  // Swap out the branch to a auth+branch variant
  auto BMI = BuildMI(MBB, *MIi, MIi->getDebugLoc(), TII->get(AArch64::BLRAA));
  BMI.add(MIi->getOperand(0));
  BMI.addReg(PARTS::getModifierReg());

  // Remove the old instruction!
  auto &MI = *MIi;
  MIi--;
  MI.removeFromParent();

  return true;
}

bool PartsPassPointerLoadStore::instrumentDataPointerStore(MachineBasicBlock &MBB, MachineInstr &MI,
                                                           unsigned pointerReg, type_id_t type_id)
{
  return false;
}

bool PartsPassPointerLoadStore::instrumentDataPointerLoad(MachineBasicBlock &MBB, MachineInstr &MI,
                                                          unsigned pointerReg, type_id_t type_id)
{
  return false;
}
