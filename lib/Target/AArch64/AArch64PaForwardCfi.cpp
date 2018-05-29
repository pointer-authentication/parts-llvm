/*
 * AArch64PaForwardCfi.cpp
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

        bool isLoad(MachineInstr &MI);
        bool isStore(MachineInstr &MI);
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

    unsigned opcode = 0;
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

            if (isLoad(MI)) {
                errs() << "found " << TII->getName(MI.getOpcode()) << "\n";
            } else if (isStore(MI)) {

                /* auto &C = MF.getFunction().getContext(); */
                /* MDNode *N = MDNode::get(C, MDString::get(C, "my md string content")); */
                /* MI.setMetadata(N); */

                errs() << "found " << TII->getName(MI.getOpcode())
                    << ", with " << MI.getNumOperands() << " operands"
                    << "\n";

                MI.dump();

                auto op2 = MI.getOperand(2);

            } else {
                errs() << "unable to identify MemOp type\n";
            }
        }
    }

    return true;
}

bool PaForwardCfi::isStore(MachineInstr &MI) {
    switch(MI.getOpcode()) {
    default:
        return false;
    case AArch64::STRWpost:
    case AArch64::STURQi:
    case AArch64::STURXi:
    case AArch64::STURDi:
    case AArch64::STURWi:
    case AArch64::STURSi:
    case AArch64::STURHi:
    case AArch64::STURHHi:
    case AArch64::STURBi:
    case AArch64::STURBBi:
    case AArch64::STPQi:
    case AArch64::STNPQi:
    case AArch64::STRQui:
    case AArch64::STPXi:
    case AArch64::STPDi:
    case AArch64::STNPXi:
    case AArch64::STNPDi:
    case AArch64::STRXui:
    case AArch64::STRDui:
    case AArch64::STPWi:
    case AArch64::STPSi:
    case AArch64::STNPWi:
    case AArch64::STNPSi:
    case AArch64::STRWui:
    case AArch64::STRSui:
    case AArch64::STRHui:
    case AArch64::STRHHui:
    case AArch64::STRBui:
    case AArch64::STRBBui:
        return true;
    }
}

bool PaForwardCfi::isLoad(MachineInstr &MI) {
    switch(MI.getOpcode()) {
    default:
        return false;
    case AArch64::LDPXi:
    case AArch64::LDPDi:
    case AArch64::LDRWpost:
    case AArch64::LDURQi:
    case AArch64::LDURXi:
    case AArch64::LDURDi:
    case AArch64::LDURWi:
    case AArch64::LDURSi:
    case AArch64::LDURSWi:
    case AArch64::LDURHi:
    case AArch64::LDURHHi:
    case AArch64::LDURSHXi:
    case AArch64::LDURSHWi:
    case AArch64::LDURBi:
    case AArch64::LDURBBi:
    case AArch64::LDURSBXi:
    case AArch64::LDURSBWi:
    case AArch64::LDPQi:
    case AArch64::LDNPQi:
    case AArch64::LDRQui:
    case AArch64::LDNPXi:
    case AArch64::LDNPDi:
    case AArch64::LDRXui:
    case AArch64::LDRDui:
    case AArch64::LDPWi:
    case AArch64::LDPSi:
    case AArch64::LDNPWi:
    case AArch64::LDNPSi:
    case AArch64::LDRWui:
    case AArch64::LDRSui:
    case AArch64::LDRSWui:
    case AArch64::LDRHui:
    case AArch64::LDRHHui:
    case AArch64::LDRBui:
    case AArch64::LDRBBui:
        return true;
    }
}
