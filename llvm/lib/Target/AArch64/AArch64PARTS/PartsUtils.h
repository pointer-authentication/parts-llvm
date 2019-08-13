//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
//         Gilang Mentari Hamidy <gilang.hamidy@gmail.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
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

namespace llvm {

namespace PARTS {

	using type_id_t = uint64_t;

class PartsUtils;

typedef std::shared_ptr<PartsUtils> PartsUtils_ptr;

static inline unsigned getModifierReg() { return AArch64::X23; }

class PartsUtils {
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

  /* UNUSED
  PartsTypeMetadata inferPauthTypeIdRegBackwards(MachineFunction &MF,
                                                     MachineBasicBlock &MBB,
                                                     MachineInstr &MI,
                                                     unsigned targetReg);



  PartsTypeMetadata inferPauthTypeIdStackBackwards(MachineFunction &MF,
                                                       MachineBasicBlock &MBB,
                                                       MachineInstr &MI,
                                                       unsigned targetReg, unsigned reg, int64_t imm);

   

  void attach(LLVMContext &C, PartsTypeMetadata PTMD, MachineInstr *MI);

  void attach(LLVMContext &C, PartsTypeMetadata PTMD,
              MachineInstrBuilder *MIB);
                           */

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

  void convertPartIntrinsic(MachineBasicBlock &MBB, MachineInstr &MI, unsigned instr);

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
