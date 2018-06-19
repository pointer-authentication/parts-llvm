/*
 * PointerAuthentication.h
 * Copyright (C) 2018 Hans Liljestrand <hans.liljestrand@pm.me>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef POINTERAUTHENTICATION_H
#define POINTERAUTHENTICATION_H

#include <string>
#include "AArch64Subtarget.h"
#include "llvm/CodeGen/MachineInstr.h"
/* #include <llvm/MachineInstr.h> */

namespace llvm {
    namespace PA {

        const std::string MDKind = "PAData";

        bool isLoad(MachineInstr &MI);
        bool isStore(MachineInstr &MI);
        const MDNode *getPAData(MachineInstr &MI);
        bool isInstrPointer(const MDNode *paData);
    }
}

#endif /* !POINTERAUTHENTICATION_H */
