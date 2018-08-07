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
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetLowering.h"

#define ENABLE_PAUTH_SLLOW true

//#define DISABLE_PA_DEBUG 1
//#define DEBUG_ONLY_FUNC "check_user"
//#define DEBUG_ONLY_FUNC "DivideInternalFPF"
#define DEBUG_PA_OPT(F,x) do { \
  x;\
  errs() << KNRM; \
} while(0);
#define DEBUG_PA_MIR(F,x) do { \
  x;\
  errs() << KNRM; \
} while(0);
#define DEBUG_PA_LOW(F,x) do { \
  x;\
  errs() << KNRM; \
} while(0);
#define DEBUG_PA_FUNC(F, name, x) do { \
  if (((F)->getName() == (name)) { \
    errs() << KBLU; \
    x; \
    errs() << KNRM; \
  } \
} while(0);


#ifdef DEBUG_ONLY_FUNC
#undef DEBUG_PA_OPT
#undef DEBUG_PA_MIR
#undef DEBUG_PA_LOW
#undef DEBUG_PA_FUNC
#define DEBUG_PA_OPT(F,x) do { \
  if (F != nullptr && DEBUG_ONLY_FUNC == (F)->getName()) { \
    x;\
    errs() << KNRM; \
  } \
} while(0);
#define DEBUG_PA_MIR(F,x) do { \
  if (F != nullptr && DEBUG_ONLY_FUNC == (F)->getName()) { \
    x;\
    errs() << KNRM; \
  } \
} while(0);
#define DEBUG_PA_LOW(F,x) do { \
  if (F != nullptr && DEBUG_ONLY_FUNC == (F)->getName()) { \
    x;\
    errs() << KNRM; \
  } \
} while(0);
#define DEBUG_PA_FUNC(F, name, x) do { \
  if (DEBUG_ONLY_FUNC == (F)->getName()) { \
    errs() << KBLU; \
    x; \
    errs() << KNRM; \
  } \
} while(0);
#endif

#ifdef DISABLE_PA_DEBUG
#undef DEBUG_PA_OPT
#define DEBUG_PA_OPT(F,x)
#undef DEBUG_PA_MIR
#define DEBUG_PA_MIR(F,x)
#undef DEBUG_PA_LOW
#define DEBUG_PA_LOW(F,x)
#undef DEBUG_PA_FUNC
#define DEBUG_PA_FUNC(F,x)
#endif

#define Pauth_ModifierReg AArch64::X23

namespace llvm {
namespace PA {

/*
class TypeID : public MDNode {
public:
  uint64_t

  uint64_t type_id = 0;
  bool unknown = true;
  bool pointer = false;
  bool data = false;
};
*/

typedef uint64_t pauth_type_id;
typedef uint64_t pauth_function_id;

const std::string Pauth_MDKind = "PAData";

static constexpr uint64_t type_id_mask_found = 1U; /* this is a found but ignored pointer */
static constexpr uint64_t type_id_mask_ptr = 2U; /* is this a pointer (zero for nope)*/
static constexpr uint64_t type_id_mask_instr = 4U; /* is this a instruction pointer */
static constexpr uint64_t type_id_mask_funcHasDef = 8U; /* is this a instruction pointer */

static constexpr pauth_type_id type_id_Unknown = 0;
static constexpr pauth_type_id type_id_Ignore = type_id_mask_found;

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


pauth_type_id createPauthTypeId(const Type *const Ty);
pauth_type_id getPauthTypeId(MachineInstr &MI);
pauth_type_id getPauthTypeId(const MDNode *PAMDNode);
pauth_type_id getPauthTypeId(const Constant *C);

bool isPauthMDNode(const MDNode *PAMDNode);
bool isPauthMDNode(const MachineOperand &op);
Constant *getPauthTypeIdConstant(const MDNode *MDNode);

MDNode *createPauthMDNode(LLVMContext &C, pauth_type_id type_id);
MDNode *createPauthMDNode(LLVMContext &C, const Type *Ty);

void addPauthMDNode(LLVMContext &C, MachineInstr &MI, pauth_type_id id);
void addPauthMDNode(MachineInstr &MI, MDNode node);

bool isInstrPointer(const MDNode *paData);

void addPAMDNodeToCall(LLVMContext &C, MachineInstrBuilder &MIB, const Instruction *F);
void addPAMDNodeToCall(LLVMContext &C, MachineInstrBuilder &MIB, llvm::FastISel::CallLoweringInfo &CLI);
void addPAMDNodeToCall(LLVMContext &C, MachineInstrBuilder &MIB, pauth_type_id type_id);

inline bool isUnknown(const pauth_type_id &type_id);
inline bool isPointer(const pauth_type_id &type_id);
inline bool isInstruction(const pauth_type_id &type_id);
inline bool isIgnored(const pauth_type_id &type_id);
inline bool isDirectFunc(const pauth_type_id &type_id);
inline bool isData(const pauth_type_id &id);

}
}

using namespace llvm;

inline bool PA::isUnknown(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_found) == 0;
}

inline bool PA::isIgnored(const pauth_type_id &type_id)
{
  return type_id == type_id_Ignore;
}

inline bool PA::isPointer(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_ptr) != 0;
}

inline bool PA::isInstruction(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_instr) != 0;
}

inline bool PA::isData(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_instr) == 0 && !isInstruction(type_id);
}

inline bool PA::isDirectFunc(const pauth_type_id &type_id)
{
  return (type_id & type_id_mask_funcHasDef) != 0;
}

#endif /* !POINTERAUTHENTICATION_H */
