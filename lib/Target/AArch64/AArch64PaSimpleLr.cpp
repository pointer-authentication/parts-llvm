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

        StringRef getPassName() const override { return "Stupid tester by Hans"; }

        bool doInitialization(Module &M) override;
        bool runOnMachineFunction(MachineFunction &F) override;

        void insertEpiloguePac(MachineBasicBlock &BB);
        void insertReturnPac(MachineBasicBlock &BB);

        /* void getAnalysisUsage(AnalysisUsage &AU) const override { */
        /*   MachineFunctionPass::getAnalysisUsage(AU); */
        /*   AU.addRequired<MachineModuleInfo>(); */
        /*   AU.addPreserved<MachineModuleInfo>(); */
        /* } */

    private:
        /* MachineModuleInfo *MMI; */
        /* bool Is64Bit; */
        const TargetMachine *TM;
        const AArch64Subtarget *STI;
        const AArch64InstrInfo *TII;
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

    std::cerr << "compiling " << MF.getName().str() << " \n";
    bool first = true;

    /* for (const MachineBasicBlock &BB : MF) { */
    for (const auto &BB : MF) {
        std::cerr << "block: " << BB.getName().str() << "\n";

        if (first) {
            std::cerr << "found first block\n";
            first = false;
        }

        if (BB.isReturnBlock()) {
            std::cerr << "found return block\n";
        }

    }

    return true;
}

void PaSimpleLr::insertEpiloguePac(MachineBasicBlock &BB)
{
    /* auto first = BB.begin(); */

    /* BuildMI(BB, first, DeubLogic(), */ 
    /*         TTI.get(TargetOpcode:: */

    std::cerr << __FUNCTION__ << "\n";
}

void PaSimpleLr::insertReturnPac(MachineBasicBlock &BB)
{
    std::cerr << __FUNCTION__ << "\n";
}
