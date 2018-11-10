//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_PARTSUTILS_H
#define LLVM_PARTSUTILS_H

#include <memory>
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "AArch64.h"
#include "AArch64RegisterInfo.h"
#include "AArch64InstrInfo.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/PARTS/PartsLog.h"

namespace llvm {

namespace PARTS {

class PartsUtils;

typedef std::shared_ptr<PartsUtils> PartsUtils_ptr;

static inline unsigned getModifierReg() { return AArch64::X23; }

class PartsUtils {
  PartsLog_ptr log;

  const TargetInstrInfo *TII;
  const TargetRegisterInfo *TRI;

  PartsUtils() = delete;

public:
  PartsUtils(const TargetRegisterInfo *TRI, const TargetInstrInfo *TII);

  static inline PartsUtils_ptr get(const TargetRegisterInfo *TRI, const TargetInstrInfo *TII) {
    return std::make_shared<PartsUtils>(TRI, TII);
  };

  inline bool registerFitsPointer(unsigned reg);

  inline bool checkIfRegInstrumentable(unsigned reg);

  PartsTypeMetadata_ptr inferPauthTypeIdRegBackwards(MachineFunction &MF,
                                                     MachineBasicBlock &MBB,
                                                     MachineInstr &MI,
                                                     unsigned targetReg);

  PartsTypeMetadata_ptr inferPauthTypeIdStackBackwards(MachineFunction &MF,
                                                       MachineBasicBlock &MBB,
                                                       MachineInstr &MI,
                                                       unsigned targetReg, unsigned reg, int64_t imm);

  void attach(LLVMContext &C, PartsTypeMetadata_ptr PTMD, MachineInstr *MI);

  void attach(LLVMContext &C, PartsTypeMetadata_ptr PTMD, MachineInstrBuilder *MIB);

  void pacCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned dstReg,
                      unsigned srcReg, unsigned modReg, type_id_t type_id, const DebugLoc &DL);

  void pacCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                      unsigned modReg, type_id_t type_id, const DebugLoc &DL);

  void pacDataPointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                      unsigned modReg, type_id_t type_id, const DebugLoc &DL);

  void autCodePointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                      unsigned modReg, type_id_t type_id, const DebugLoc &DL);

  void autDataPointer(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                      unsigned modReg, type_id_t type_id, const DebugLoc &DL);

  bool isLoadOrStore(const MachineInstr &MI);

  bool isLoad(const MachineInstr &MI);

  bool isStore(const MachineInstr &MI);

  bool isLoad(unsigned opCode);

  bool isStore(unsigned opCode);

  void moveTypeIdToReg(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned modReg,
                       type_id_t type_id, const DebugLoc &DL);

  void moveTypeIdToReg(MachineBasicBlock &MBB, MachineInstr *MI, unsigned modReg,
                       type_id_t type_id, const DebugLoc &DL);

  void createBeCfiModifier(MachineBasicBlock &MBB, MachineInstr *MIi, unsigned modReg, const DebugLoc &DL);

  void insertPAInstr(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned ptrReg,
                     unsigned modReg, const MCInstrDesc &MCID, const DebugLoc &DL);

  void insertPAInstr(MachineBasicBlock &MBB, MachineInstr *MI, unsigned ptrReg,
                     unsigned modReg, const MCInstrDesc &MCID, const DebugLoc &DL);

  void addNops(MachineBasicBlock &MBB, MachineInstr *MI, unsigned ptrReg, unsigned modReg, const DebugLoc &DL);

  void insertPAInstr(MachineBasicBlock &MBB, MachineBasicBlock::instr_iterator MIi, unsigned srcReg,
                     unsigned dstReg, unsigned modReg, const MCInstrDesc &MCID, const DebugLoc &DL);

  void addEventCallFunction(MachineBasicBlock &MBB, MachineInstr &MI,
                                   const DebugLoc &DL, Function *func);
};

inline bool PartsUtils::registerFitsPointer(unsigned reg)
{
  const auto RC = TRI->getMinimalPhysRegClass(reg);
  if (64 <= TRI->getRegSizeInBits(*RC)) {
    return true;
  }
  return false;
}

inline bool PartsUtils::checkIfRegInstrumentable(unsigned reg)
{
  if (reg == AArch64::FP || reg == AArch64::LR) {
    return false;
  }
  return registerFitsPointer(reg);
}



} // PARTS

} // llvm

#endif //LLVM_PARTSUTILS_H
