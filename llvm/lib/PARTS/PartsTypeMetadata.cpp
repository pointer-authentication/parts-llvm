//===----------------------------------------------------------------------===//
//
// Authors: Hans Liljestrand <hans.liljestrand@pm.me>
//          Zaheer Ahmed Gauhar <zaheer.gauhar@aalto.fi>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/PARTS/Parts.h"
#include <regex>

extern "C" {
// A bit ugly, but works...
#include "../PARTS-sha3/include/sha3.h"
}

#define PARTS_USE_SHA3

using namespace llvm;

namespace {

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
    O << "a." << T->getArrayNumElements();
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

  return theTypeID;
}

} // namespace

Constant *llvm::PARTS::getTypeIDConstantFrom(const Type &T, LLVMContext &C) {
  auto typeID = getTypeIDFor(&T);
  return Constant::getIntegerValue(Type::getInt64Ty(C), APInt(64, typeID));
}

