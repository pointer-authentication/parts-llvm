//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
//         Carlos Chinea <carlos.chinea.perez@huawei.com>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
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

static cl::opt<PARTS::PartsFeCfiType> PartsFeCfi(
    "parts-cpi", cl::init(PartsFeCfiNone),
    cl::desc("PARTS code-pointer protection"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(PartsFeCfiNone, "none",
                          "No PARTS code-pointer protection"),
               clEnumValN(PartsFeCfiFull, "full",
                          "PARTS code-pointer protection using type-id"),
               clEnumValN(PartsFeCfiFullNoType, "notype",
                          "Code-pointer protection with 0x0 modifier")));

static cl::opt<PARTS::PartsDpiType> PartsDpi(
    "parts-dpp", cl::init(PartsDpiNone),
    cl::desc("PARTS data-pointer protection"),
    cl::value_desc("mode"),
    cl::values(clEnumValN(PartsDpiNone, "none",
                          "No PARTS data-pointer protection"),
               clEnumValN(PartsDpiFull, "full",
                          "PARTS data-pointer protection using type-id"),
               clEnumValN(PartsDpiFullNoType, "notype",
                          "Data-pointer protection with 0x0 modifier")));

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


void llvm::PARTS::setPartsDpiUnionTypePunning(bool value) {
  EnablePartsDpiUnionTypePunning = value;
}

bool llvm::PARTS::useBeCfi() {
  return PartsBeCfi != PartsBeCfiNone;
}

bool llvm::PARTS::useFeCfi() {
  return EnablePartsFeCfi || (PartsFeCfi != PartsFeCfiNone);
}

bool llvm::PARTS::useDpi() {
  return EnablePartsDpi || (PartsDpi != PartsDpiNone);
}

bool llvm::PARTS::isUnionTypePunningSupported() {
  return EnablePartsDpiUnionTypePunning;
}

bool llvm::PARTS::useAny() {
  return useDpi() || useFeCfi() || useBeCfi();
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

PartsFeCfiType PARTS::getFeCfiType() {
  return PartsFeCfi;
}

PartsDpiType PARTS::getDpiType() {
  return PartsDpi;
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

static inline bool isCodePointer(const Type *const type) {
  return type->isPointerTy() && type->getPointerElementType()->isFunctionTy();
}

static inline bool isDataPointer(const Type *const type) {
  return type->isPointerTy() && !type->getPointerElementType()->isFunctionTy();
}

} // namespace

Constant *PARTS::getTypeIDConstantFrom(const Type &T, LLVMContext &C) {
  if (PartsFeCfi == PartsFeCfiFullNoType && isCodePointer(&T)) {
    static auto *zero =  Constant::getIntegerValue(Type::getInt64Ty(C),
                                                   APInt(64, 0));
    return zero;
  }

  if (PartsDpi == PartsDpiFullNoType && isDataPointer(&T)) {
    static auto *zero = Constant::getIntegerValue(Type::getInt64Ty(C),
                                                  APInt(64, 0));
    return zero;
  }

  return Constant::getIntegerValue(Type::getInt64Ty(C),
                                   APInt(64, getTypeIDFor(&T)));
}
