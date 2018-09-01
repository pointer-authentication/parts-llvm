//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef POINTERAUTHENTICATION_H
#define POINTERAUTHENTICATION_H

#include <string>
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/PARTS/PartsLog.h"

#define ENABLE_PAUTH_SLLOW true

#define Pauth_ModifierReg AArch64::X23

namespace llvm {
namespace PA {

typedef uint64_t pauth_type_id;
typedef uint64_t pauth_function_id;

const std::string Pauth_MDKind = "PAData";

static constexpr uint64_t type_id_mask_found = 1U; /* this is a found but ignored pointer */
static constexpr uint64_t type_id_mask_ptr = 2U; /* is this a pointer (zero for nope)*/
static constexpr uint64_t type_id_mask_instr = 4U; /* is this a instruction pointer */
static constexpr uint64_t type_id_mask_funcHasDef = 8U; /* is this a instruction pointer */
static constexpr uint64_t type_id_mask_ignore = 16U; /* this is a found but ignored pointer */

static constexpr pauth_type_id type_id_Unknown = 0;
static constexpr pauth_type_id type_id_Ignore = type_id_mask_found | type_id_mask_ignore;

bool isLoad(MachineInstr &MI);
bool isStore(MachineInstr &MI);
bool isLoad(MachineInstr *MI);
bool isStore(MachineInstr *MI);
const MDNode *getPAData(MachineInstr &MI);
const MDNode *getPAData(const MDNode &n);
const MDNode *getPAData(const MDNode *n);

void buildPAC(const TargetInstrInfo &TII,
              MachineBasicBlock &MBB, MachineBasicBlock::iterator iter,
              const DebugLoc &DL, unsigned ctxReg, unsigned ptrReg);

void instrumentEpilogue(const TargetInstrInfo *TII,
                        MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                        const DebugLoc &DL, bool IsTailCallReturn);


pauth_type_id createPauthTypeId(const Type *Ty);
pauth_type_id createPauthTypeId(const Instruction &I, const Type *Ty);
pauth_type_id getPauthTypeId(MachineInstr &MI);
pauth_type_id getPauthTypeId(const MDNode *PAMDNode);
pauth_type_id getPauthTypeId(const Constant *C);

bool isPauthMDNode(const MDNode *PAMDNode);
bool isPauthMDNode(const MachineOperand &op);
Constant *getPauthTypeIdConstant(const MDNode *MDNode);

MDNode *createPauthMDNode(LLVMContext &C, pauth_type_id type_id);
MDNode *createPauthMDNode(LLVMContext &C, const Instruction &I, const Type *Ty);
MDNode *createPauthMDNode(LLVMContext &C, const Type *Ty);

void addPauthMDNode(LLVMContext &C, MachineInstr &MI, pauth_type_id id);
void addPauthMDNode(MachineInstr &MI, MDNode node);

bool isInstrPointer(const MDNode *paData);

void addPAMDNodeToCall(LLVMContext &C, MachineInstrBuilder &MIB, const Instruction *F);
void addPAMDNodeToCall(LLVMContext &C, MachineInstrBuilder &MIB, llvm::FastISel::CallLoweringInfo &CLI);
void addPAMDNodeToCall(LLVMContext &C, MachineInstrBuilder &MIB, pauth_type_id type_id);

inline bool isUnknown(const pauth_type_id &type_id);
inline bool isPointer(const pauth_type_id &type_id);
inline bool isCodePointer(const pauth_type_id &type_id);
inline bool isDataPointer(const pauth_type_id &id);
inline bool isIgnored(const pauth_type_id &type_id);
inline bool isDirectFunc(const pauth_type_id &type_id);

inline void setTypeIdFlag(pauth_type_id &type_id, pauth_type_id mask, bool val);
inline void setIsIgnored(pauth_type_id &type_id, bool val);
inline void setIsFound(pauth_type_id &type_id, bool val);
inline void setIsPointer(pauth_type_id &type_id, bool val);
inline void setIsDataPointer(pauth_type_id &type_id, bool val);
inline void setIsCodePointer(pauth_type_id &type_id, bool val);

}
}

using namespace llvm;

inline bool PA::isUnknown(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_found) == 0;
}

inline bool PA::isIgnored(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_ignore) != 0;
}

inline bool PA::isPointer(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_ptr) != 0;
}

inline bool PA::isCodePointer(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_instr) != 0;
}

inline bool PA::isDataPointer(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_instr) == 0 && !isCodePointer(type_id);
}

inline bool PA::isDirectFunc(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_funcHasDef) != 0;
}

inline void PA::setTypeIdFlag(pauth_type_id &type_id, const pauth_type_id mask, const bool val)
{
  if (val) {
    type_id = type_id | mask;
  } else {
    type_id = type_id & ~mask;
  }
}

inline void PA::setIsIgnored(pauth_type_id &type_id, const bool val)
{
  setTypeIdFlag(type_id, type_id_mask_ignore, val);
}

inline void PA::setIsFound(pauth_type_id &type_id, const bool val)
{
    setTypeIdFlag(type_id, type_id_mask_found, val);
}

inline void PA::setIsPointer(pauth_type_id &type_id, const bool val)
{
  setTypeIdFlag(type_id, type_id_mask_ptr, val);
}

inline void PA::setIsDataPointer(pauth_type_id &type_id, const bool val)
{
  setTypeIdFlag(type_id, type_id_mask_instr, !val);
}

inline void PA::setIsCodePointer(pauth_type_id &type_id, const bool val)
{
  setTypeIdFlag(type_id, type_id_mask_instr, val);
}

#endif /* !POINTERAUTHENTICATION_H */
