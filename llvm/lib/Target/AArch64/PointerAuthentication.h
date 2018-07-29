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

namespace llvm {
namespace PA {

typedef uint32_t pauth_type_id;

const std::string Pauth_MDKind = "PAData";

static constexpr uint32_t type_id_mask_ptr = 1U; /* is this a pointer (zero for nope)*/
static constexpr uint32_t type_id_mask_instr = 2U; /* is this a instruction pointer */

bool isLoad(MachineInstr &MI);
bool isStore(MachineInstr &MI);
const MDNode *getPAData(MachineInstr &MI);
bool isInstrPointer(const MDNode *paData);

void buildPAC(const TargetInstrInfo &TII,
              MachineBasicBlock &MBB, MachineBasicBlock::iterator iter,
              const DebugLoc &DL, unsigned ctxReg, unsigned ptrReg);

void instrumentEpilogue(const TargetInstrInfo *TII,
                        MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                        const DebugLoc &DL, bool IsTailCallReturn);

pauth_type_id getPauthType(const Type *Ty);
pauth_type_id getPauthType(const MDNode *PAMDNode);
pauth_type_id getPauthType(const Constant *C);
Constant *getPauthTypeConstant(const MDNode *MDNode);

MDNode *getPauthMDNode(LLVMContext &C, const Type *Ty);

inline bool isPointer(const pauth_type_id &id) { return id != 0; }
inline bool isInstruction(const pauth_type_id &id) { return (id ^ type_id_mask_instr) == 1; }
inline bool isData(const pauth_type_id &id) { return (id ^ type_id_mask_instr) == 0; }

}
}

#endif /* !POINTERAUTHENTICATION_H */
