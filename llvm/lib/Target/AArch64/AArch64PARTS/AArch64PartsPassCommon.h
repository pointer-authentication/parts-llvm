//===----------------------------------------------------------------------===//
//
// Authors: Hans Liljestrand <hans@liljestrand.dev>
//         Carlos Chinea <carlos.chinea.perez@huawei.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __AARCH64PARTSOPTPASS_H__
#define __AARCH64PARTSOPTPASS_H__

#include "AArch64InstrInfo.h"
#include "AArch64Subtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/PARTS/Parts.h"

using namespace llvm;
using namespace llvm::PARTS;

namespace llvm {
namespace PARTS {

class AArch64PartsPassCommon {
protected:
  inline void initRunOn(MachineFunction &MF);

  inline bool hasNoPartsAttribute(MachineFunction &MF);

  inline void lowerPartsIntrinsic(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI, const MCInstrDesc &InstrDesc);
  inline void insertMovInstr(MachineBasicBlock &MBB, MachineInstr *MI, unsigned dstReg, unsigned srcReg);
  inline void replacePartsIntrinsic(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI, const MCInstrDesc &InstrDesc);
  inline void replacePartsXPACDIntrinsic(MachineFunction &MF, MachineBasicBlock &MBB, MachineInstr &MI);

  const TargetMachine *TM = nullptr;
  const AArch64Subtarget *STI = nullptr;
  const AArch64InstrInfo *TII = nullptr;
  const AArch64RegisterInfo *TRI = nullptr;
public:
  static inline void insertPACInstr(MachineBasicBlock &MBB, MachineInstr *MI, unsigned dstReg,
                                    unsigned modReg, const MCInstrDesc &InstrDesc);
};

}
}

inline void AArch64PartsPassCommon::initRunOn(MachineFunction &MF) {
  TM = &MF.getTarget();;
  STI = &MF.getSubtarget<AArch64Subtarget>();
  TII = STI->getInstrInfo();
  TRI = STI->getRegisterInfo();
}

inline bool AArch64PartsPassCommon::hasNoPartsAttribute(MachineFunction &MF) {
  return MF.getFunction().getFnAttribute("no-parts").getValueAsString() == "true";
}

inline void AArch64PartsPassCommon::lowerPartsIntrinsic(MachineFunction &MF,
                                                        MachineBasicBlock &MBB,
                                                        MachineInstr &MI,
                                                        const MCInstrDesc &InstrDesc) {
  const unsigned mod = MI.getOperand(2).getReg();
  const unsigned dst = MI.getOperand(0).getReg();

  insertPACInstr(MBB, &MI, dst, mod, InstrDesc);
}

inline void AArch64PartsPassCommon::replacePartsIntrinsic(MachineFunction &MF,
                                                          MachineBasicBlock &MBB,
                                                          MachineInstr &MI,
                                                          const MCInstrDesc &InstrDesc) {
  lowerPartsIntrinsic(MF, MBB, MI, InstrDesc);
  MI.removeFromParent();
}

inline void AArch64PartsPassCommon::insertPACInstr(MachineBasicBlock &MBB,
                                                   MachineInstr *MI,
                                                   unsigned dstReg,
                                                   unsigned modReg,
                                                   const MCInstrDesc &InstrDesc) {
  if (MI != nullptr)
    BuildMI(MBB, MI, MI->getDebugLoc(), InstrDesc, dstReg)
        .addUse(modReg);
  else
    BuildMI(&MBB, DebugLoc(), InstrDesc, dstReg)
        .addUse(modReg);
}

inline void AArch64PartsPassCommon::insertMovInstr(MachineBasicBlock &MBB,
                                                   MachineInstr *MI,
                                                   unsigned dstReg,
                                                   unsigned srcReg) {
  BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(AArch64::ORRXrs), dstReg)
      .addUse(AArch64::XZR)
      .addUse(srcReg)
      .addImm(0);
}

inline void AArch64PartsPassCommon::replacePartsXPACDIntrinsic(MachineFunction &MF,
                                                          MachineBasicBlock &MBB,
                                                          MachineInstr &MI) {
  const unsigned dst = MI.getOperand(0).getReg();
  BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(AArch64::XPACD), dst);
  MI.removeFromParent();
}

#endif // __AARCH64PARTSOPTPASS_H__
