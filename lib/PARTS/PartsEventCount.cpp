//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@aalto.fi>
// Copyright (c) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/PARTS/PartsEventCount.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/PARTS/Parts.h"

namespace llvm {

namespace PARTS {

Function *PartsEventCount::getCounterFunc(Module &M, const std::string &fName) {
  if (auto f = M.getFunction(fName)) {
    assert(f != nullptr);
    return f;
  }

  auto &C = M.getContext();

  auto result = Type::getVoidTy(C);
  FunctionType *signature = FunctionType::get(result, false);

  auto f = Function::Create(signature, Function::ExternalLinkage, fName, &M);
  f->addFnAttr("no-parts", "true");

  return f;
}

}

}
