/*
 * AArch64PaSimpleLr.cpp
 * Copyright (C) 2018 Secure Systems Group, Aalto University, ssg.aalto.fi
 * Author: Hans Liljestrand <liljestrandh@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */


#include <iostream>
#include "AArch64.h"
/* #include "AArch64InstrBuilder.h" */
#include "AArch64Subtarget.h"
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

#define DEBUG_TYPE "aarch64-pa-simplelr"

using namespace llvm;

namespace {
    class PaSimpleLr : public MachineFunctionPass {
    public:
        static char ID;

        PaSimpleLr() : MachineFunctionPass(ID) {}

        StringRef getPassName() const override { return "pa-simplelr"; }

        bool doInitialization(Module &M) override;
        bool runOnMachineFunction(MachineFunction &) override;

        void insertProloguePac(MachineBasicBlock &BB);
        void insertReturnPac(MachineBasicBlock &BB);

    private:
        /* MachineModuleInfo *MMI; */
        /* bool Is64Bit; */
        const TargetMachine *TM = nullptr;
        const AArch64Subtarget *STI = nullptr;
        const AArch64InstrInfo *TII = nullptr;
    };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PaSimpleLrPass() {
    return new PaSimpleLr();
}

char PaSimpleLr::ID = 0;

bool PaSimpleLr::doInitialization(Module &M) {
    return false;
}

bool PaSimpleLr::runOnMachineFunction(MachineFunction &MF) {
    DEBUG(dbgs() << getPassName() << '\n');

    TM = &MF.getTarget();;
    STI = &MF.getSubtarget<AArch64Subtarget>();
    TII = STI->getInstrInfo();

    insertProloguePac(MF.front());

    for (auto &BB : MF) {
        if (BB.isReturnBlock()) {
            insertReturnPac(BB);
        }
    }

    return true;
}

void PaSimpleLr::insertProloguePac(MachineBasicBlock &BB)
{
    auto first = BB.begin();
    BuildMI(BB, first, DebugLoc(), TII->get(AArch64::PACIASP));
}

void PaSimpleLr::insertReturnPac(MachineBasicBlock &BB)
{
    auto pos = --(BB.end());

    if (!pos->isReturn()) {
        DEBUG(dbgs() << "Failed to locate return instruction, trying to recover\n");
        // FIXME: Try scrolling back until return is found?
    }

    assert(pos->isReturn() && "Failed to locate return instruction");
    BuildMI(BB, pos, DebugLoc(), TII->get(AArch64::AUTIASP));
}
