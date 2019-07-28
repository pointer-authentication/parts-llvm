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
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include <regex>
#include <map>

#define PARTS_USE_SHA3

extern "C" {
#include "../PARTS-sha3/include/sha3.h"
}

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

bool llvm::PARTS::isUnionTypePunningSupported() {
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

PartsBeCfiType PARTS::getBeCfiType() {
  return PartsBeCfi;
}

namespace {

std::map<const Type *, uint64_t> TypeIDCache;

// Build string representation of the Type to llvm::raw_string_ostream
void buildTypeString(const Type *T, llvm::raw_string_ostream &O) {
  if (T->isPointerTy()) {
    O << "ptr.";
    buildTypeString(T->getPointerElementType(), O);
  } else if (T->isStructTy()) {
    auto structName = dyn_cast<StructType>(T)->getStructName();
    std::regex e("^(\\w+\\.\\w+)(\\.\\w+)?$");
    O << std::regex_replace(structName.str(), e, "$1");
  } else if (T->isArrayTy()) {
    O << "ptr.";
    buildTypeString(T->getArrayElementType(), O);
  } else if (T->isFunctionTy()) {
    auto FuncTy = dyn_cast<FunctionType>(T);
    O << "f.";
    buildTypeString(FuncTy->getReturnType(), O);

    for (auto p = FuncTy->param_begin(); p != FuncTy->param_end(); p++) {
      buildTypeString(*p, O);
    }
  } else if (T->isVectorTy()) {
    O << "vec." << T->getVectorNumElements();
    buildTypeString(T->getVectorElementType(), O);
  } else if (T->isVoidTy()) {
    O << "v";
  } else {
    /* Make sure we've handled all cases we want to */
    assert(T->isIntegerTy() || T->isFloatingPointTy());
    T->print(O);
  }
}

// Get a TypeID from given Type
uint64_t getTypeIDFor(const Type *T) {
  if (!T->isPointerTy())
    return 0; // Not a pointer, hence no type ID for this one

  // TODO: This should perform caching, so calling the same Type will not
  // reprocess the stuff. Use a Dictionary-like ADT is suggested.
  decltype(TypeIDCache)::iterator id;
  if ((id = TypeIDCache.find(T)) != TypeIDCache.end())
    return id->second;

  uint64_t theTypeID = 0;
  std::string buf;
  llvm::raw_string_ostream typeIdStr(buf);

  buildTypeString(T, typeIdStr);
  typeIdStr.flush();

  // Prepare SHA3 generation
  auto rawBuf = buf.c_str();
  mbedtls_sha3_context sha3_context;
  mbedtls_sha3_type_t sha3_type = MBEDTLS_SHA3_256;
  mbedtls_sha3_init(&sha3_context);

  // Prepare input and output variables
  auto *input = reinterpret_cast<const unsigned char *>(rawBuf);
  auto *output = new unsigned char[32]();

  // Generate hash
  auto result = mbedtls_sha3(input, buf.length(), sha3_type, output);
  if (result != 0)
    llvm_unreachable("SHA3 hashing failed :(");
  memcpy(&theTypeID, output, sizeof(theTypeID));
  delete[] output;

  TypeIDCache.emplace(T, theTypeID);

  return theTypeID;
}

} // namespace

Constant *PARTS::getTypeIDConstantFrom(const Type &T, LLVMContext &C) {
  auto typeID = getTypeIDFor(&T);
  return Constant::getIntegerValue(Type::getInt64Ty(C), APInt(64, typeID));
}
