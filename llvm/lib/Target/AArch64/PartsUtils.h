//
// Created by ishkamiel on 30/08/18.
//

#ifndef LLVM_PARTSUTILS_H
#define LLVM_PARTSUTILS_H

#include <memory>
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "AArch64.h"
#include "AArch64RegisterInfo.h"
#include "AArch64InstrInfo.h"
#include "llvm/PARTS/PartsTypeMetadata.h"

namespace llvm {

namespace PARTS {

class PartsUtils;

typedef std::shared_ptr<PartsUtils> PartsUtils_ptr;

static inline unsigned getModifierReg() { return AArch64::X23; }

class PartsUtils {

  const AArch64InstrInfo *TII;
  const AArch64RegisterInfo *TRI;

public:
  PartsUtils(const AArch64RegisterInfo *TRI, const AArch64InstrInfo *TII);

  static inline PartsUtils_ptr get(const AArch64RegisterInfo *TRI, const AArch64InstrInfo *TII) {
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
                      unsigned srcReg, unsigned modReg, type_id_t type_id);

  bool isLoadOrStore(const MachineInstr &MI);
  bool isLoad(const MachineInstr &MI);
  bool isStore(const MachineInstr &MI);
  bool isLoad(unsigned opCode);
  bool isStore(unsigned opCode);
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
