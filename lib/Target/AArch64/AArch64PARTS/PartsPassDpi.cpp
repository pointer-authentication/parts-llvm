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

#define DEBUG_TYPE "aarch64-parts-dpi"

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
 class PartsPassDpi : public MachineFunctionPass {

 public:
   static char ID;

   PartsPassDpi() :
       MachineFunctionPass(ID),
       log(PARTS::PartsLog::getLogger(DEBUG_TYPE))
   {
     DEBUG_PA(log->enable());
   }

   StringRef getPassName() const override { return DEBUG_TYPE; }

   bool doInitialization(Module &M) override;
   bool runOnMachineFunction(MachineFunction &) override;
   bool instrumentLoadStore(MachineFunction &MF, MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator &MIi);
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

FunctionPass *llvm::createPartsPassDpi() {
  return new PartsPassDpi();
}

char PartsPassDpi::ID = 0;

bool PartsPassDpi::doInitialization(Module &M) {
  return false;
}

bool PartsPassDpi::runOnMachineFunction(MachineFunction &MF) {
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

      if (partsUtils->isLoadOrStore(*MIi)) {
        instrumentLoadStore(MF, MBB, MIi);
      }
    }
  }

  return true;
}

bool PartsPassDpi::instrumentLoadStore(MachineFunction &MF, MachineBasicBlock &MBB,
                                         MachineBasicBlock::instr_iterator &MIi) {
  assert(partsUtils->isLoadOrStore(*MIi));

  auto partsType = PartsTypeMetadata::retrieve(*MIi);
  auto &C = MF.getFunction().getContext();

  DEBUG_PA(log->debug(MF.getName()) << "      found a load/store (" << TII->getName(MIi->getOpcode()) << ")\n");

  if (partsType == nullptr) {
    DEBUG_PA(log->debug(MF.getName()) << "      trying to figure out type_id\n");
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
          partsType = partsUtils->inferPauthTypeIdStackBackwards(MF, MBB, *MIi, targetReg,
                                                                 MIi->getOperand(1).getReg(),
                                                                 MIi->getOperand(2).getImm());
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

  skipIfB(!partsType->isKnown(), "StoreLoad.Unknown", false, "type_id is unknown!\n");
  skipIfN(partsType->isIgnored(), "StoreLoad.Ignored", "marked as ignored, skipping!\n");
  skipIfN(!partsType->isPointer(), "StoreLoad.NotAPointer", "not a pointer, skipping!\n");

  auto reg = MIi->getOperand(0).getReg();
  const auto modReg = PARTS::getModifierReg();
  const auto type_id = partsType->getTypeId();

  if (partsUtils->isStore(*MIi)) {
    if (partsType->isDataPointer()) {
      if (PARTS::useDpi()) {
        log->inc("StoreLoad.InstrumentedDataStore", true) << "instrumenting store" << partsType->toString() << "\n";
        partsUtils->pacDataPointer(MBB, MIi, reg, modReg, type_id, MIi->getDebugLoc());
        return true;
      }
    } else {
      return false;
      if (PARTS::useFeCfi()) {
        log->inc("StoreLoad.InstrumentedCodeStore", true) << "instrumenting store" << partsType->toString() << "\n";
        partsUtils->pacCodePointer(MBB, MIi, reg, modReg, type_id, MIi->getDebugLoc());
        return true;
      }
    }
  } else {
    auto loc = MIi;
    MIi->getDebugLoc();
    loc++;
    if (partsType->isDataPointer()) {
      if (PARTS::useDpi()) {
        log->inc("StoreLoad.InstrumentedDataLoad", true) << "instrumenting load" << partsType->toString() << "\n";
        partsUtils->autDataPointer(MBB, loc, reg, modReg, type_id, MIi->getDebugLoc());
        return true;
      }
    } else {
      return false;
      if (PARTS::useFeCfi()) {
        log->inc("StoreLoad.InstrumentedCodeLoad", true) << "instrumenting load" << partsType->toString() << "\n";
        partsUtils->autCodePointer(MBB, loc, reg, modReg, type_id, MIi->getDebugLoc());
        return true;
      }
    }
  }

  return false;
}
