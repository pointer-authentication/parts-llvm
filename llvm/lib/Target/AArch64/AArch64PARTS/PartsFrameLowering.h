//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTSFRAMELOWERING_H
#define LLVM_PARTSFRAMELOWERING_H

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "AArch64InstrInfo.h"
#include "llvm/PARTS/PartsLog.h"
#include "PartsUtils.h"

namespace llvm {

class PartsFrameLowering;
typedef std::shared_ptr<PartsFrameLowering> PartsFrameLowering_ptr;

class PartsFrameLowering {
  PARTS::PartsLog_ptr log;

public:
  PartsFrameLowering() : log(PARTS::PartsLog::getLogger("PartsFrameLowering")) {
    DEBUG_PA(log->enable());
  }

  static PartsFrameLowering_ptr get();

  void instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                          MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                          const DebugLoc &DL, bool IsTailCallReturn);

  void instrumentEpilogue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                          MachineBasicBlock &MBB, const DebugLoc &DL, bool IsTailCallReturn);

  void instrumentPrologue(const TargetInstrInfo *TII, const TargetRegisterInfo *TRI,
                          MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                          const DebugLoc &DL);
};


}

#endif //LLVM_PARTSFRAMELOWERING_H
