//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/Parts.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;
using namespace PARTS;

static cl::opt<PARTS::PartsBeCfiType> PartsBeCfi(
    "parts-becfi", cl::init(PartsBeCfiNone),
    cl::desc("PARTS backward-edge CFI"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(PartsBeCfiNone, "none", "No PARTS backward-edge protection"),
               clEnumValN(PartsBeCfiFull, "full", "Full backward-edge protection"),
               clEnumValN(PartsBeCfiNgFull, "ng-full", "Full optimized backward-edge protection")));

static cl::opt<bool> EnablePartsFeCfi("parts-fecfi", cl::Hidden,
                                      cl::desc("PARTS backward-edge CFI"),
                                      cl::init(false));

static cl::opt<bool> EnablePartsDpi("parts-dpi", cl::Hidden,
                                    cl::desc("PARTS backward-edge CFI"),
                                    cl::init(false));

static cl::opt<bool> EnablePartsDpiUnionTypePunning("parts-dpi-union-type-punning", cl::Hidden,
                                    cl::desc("Disable parts when loading union data members"),
                                    cl::init(false));


static cl::opt<bool> UseDummyInstructions("parts-dummy", cl::Hidden,
                                          cl::desc("Use dummy instructions and XOR instead of PA"),
                                          cl::init(false));

static cl::opt<bool> EnablePartsRuntimeStats("parts-stats", cl::Hidden,
                                          cl::desc("Invoke stat counting functions to count various events"),
                                          cl::init(false));

bool llvm::PARTS::useBeCfi() {
  return PartsBeCfi != PartsBeCfiNone;
}

bool llvm::PARTS::useFeCfi() {
  return EnablePartsFeCfi;
}

bool llvm::PARTS::useDpi() {
  return EnablePartsDpi;
}

bool llvm::PARTS::isUnionTypePunningSupported(void) {
  return EnablePartsDpiUnionTypePunning;
}

bool llvm::PARTS::useAny() {
  return EnablePartsDpi || EnablePartsFeCfi || useBeCfi();
}

bool llvm::PARTS::useDummy() {
  return UseDummyInstructions;
}

bool llvm::PARTS::useRuntimeStats() {
  return EnablePartsRuntimeStats;
}

namespace llvm {
namespace PARTS {

PartsBeCfiType getBeCfiType() {
  return PartsBeCfi;
}

}
}

