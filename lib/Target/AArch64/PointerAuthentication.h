/*
 * PointerAuthentication.h
 * Copyright (C) 2018 Hans Liljestrand <hans.liljestrand@pm.me>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef POINTERAUTHENTICATION_H
#define POINTERAUTHENTICATION_H

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#include <string>
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineBasicBlock.h"

namespace llvm {
    namespace PA {

        const std::string MDKind = "PAData";

        bool isLoad(MachineInstr &MI);
        bool isStore(MachineInstr &MI);
        const MDNode *getPAData(MachineInstr &MI);
        bool isInstrPointer(const MDNode *paData);

        void buildPAC(const TargetInstrInfo &TII,
                      MachineBasicBlock &MBB, MachineBasicBlock::iterator iter,
                      const DebugLoc &DL, unsigned ctxReg, unsigned ptrReg);
    }
}

#endif /* !POINTERAUTHENTICATION_H */
