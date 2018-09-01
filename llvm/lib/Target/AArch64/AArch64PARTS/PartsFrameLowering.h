//
// Created by Hans Liljestrand on 15/08/18.
// Copyright (c) 2018 Hans Liljestrand. All rights reserved.
//

#ifndef LLVM_PARTSFRAMELOWERING_H
#define LLVM_PARTSFRAMELOWERING_H

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "AArch64InstrInfo.h"

namespace llvm {

class PartsFrameLowering;
typedef std::shared_ptr<PartsFrameLowering> PartsFrameLowering_ptr;

class PartsFrameLowering {
public:
  PartsFrameLowering() {}

  static PartsFrameLowering_ptr get();

  void instrumentEpilogue(const TargetInstrInfo *TII,
                          MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                          const DebugLoc &DL, bool IsTailCallReturn);

  void instrumentPrologue(const TargetInstrInfo *TII,
                          MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                          const DebugLoc &DL);
};


}

#endif //LLVM_PARTSFRAMELOWERING_H
