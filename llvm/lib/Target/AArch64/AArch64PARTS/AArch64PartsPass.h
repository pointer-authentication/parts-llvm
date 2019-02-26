//===----------------------------------------------------------------------===//
//
// Authors: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __AARCH64PARTSOPTPASS_H__
#define __AARCH64PARTSOPTPASS_H__

#include "llvm/PARTS/PartsTypeMetadata.h"

using namespace llvm;
using namespace llvm::PARTS;

namespace llvm {
namespace PARTS {

class AArch64PartsPass {
protected:
  inline bool hasNoPartsAttribute(MachineFunction &MF);
};

};
};

inline bool AArch64PartsPass::hasNoPartsAttribute(MachineFunction &MF) {
  return MF.getFunction().getFnAttribute("no-parts").getValueAsString() == "true";
}

#endif // __AARCH64PARTSOPTPASS_H__
