/*
 * AArch64PaForwardCfi.cpp
 * Copyright (C) 2018 Secure Systems Group, Aalto University, ssg.aalto.fi
 * Author: Hans Liljestrand <liljestrandh@gmail.com>
 *
 * Distributed under terms of the MIT license.
 */


#include <iostream>
#include "PointerAuthentication.h"
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

#define DEBUG_TYPE "aarch64-pa-forwardcfi"

using namespace llvm;

namespace {
    class PaForwardCfi : public MachineFunctionPass {
    public:
        static char ID;

        PaForwardCfi() : MachineFunctionPass(ID) {}

        StringRef getPassName() const override { return "pa-forwardcfi"; }

        bool doInitialization(Module &M) override;
        bool runOnMachineFunction(MachineFunction &) override;

    private:
        /* MachineModuleInfo *MMI; */
        /* bool Is64Bit; */
        const TargetMachine *TM = nullptr;
        const AArch64Subtarget *STI = nullptr;
        const AArch64InstrInfo *TII = nullptr;
    };
} // end anonymous namespace

FunctionPass *llvm::createAArch64PaForwardCfiPass() {
    return new PaForwardCfi();
}

char PaForwardCfi::ID = 0;

bool PaForwardCfi::doInitialization(Module &M) {
    return false;
}

bool PaForwardCfi::runOnMachineFunction(MachineFunction &MF) {
    DEBUG(dbgs() << getPassName() << ", function " << MF.getName() << '\n');
    errs() << "function " << MF.getName() << '\n';

    TM = &MF.getTarget();;
    STI = &MF.getSubtarget<AArch64Subtarget>();
    TII = STI->getInstrInfo();

    unsigned scale;
    unsigned width;
    int64_t min_offset;
    int64_t max_offset;

    for (auto &MBB : MF) {
        for (auto &MI : MBB) {
            if (!MI.mayLoadOrStore())
                continue;

            if (!TII->getMemOpInfo(MI.getOpcode(), scale, width, min_offset, max_offset))
                continue;

            if (PA::isLoad(MI) || PA::isStore(MI)) {
                errs() << "found " << TII->getName(MI.getOpcode())
                    << ", with " << MI.getNumOperands() << " operands"
                    << "\n";
                MI.dump();

                const MDNode *paData = PA::getPAData(MI);

                if (paData == nullptr)
                    continue;

                errs() << "Found PAData!\n";

                /* paData */

                if (PA::isLoad(MI)) {
                    // check PAC
                } else {
                    // set PAC
                }
            }
        }
    }

    return true;
}
