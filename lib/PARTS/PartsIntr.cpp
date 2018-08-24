//
// Created by Hans Liljestrand on 24/08/18.
// Copyright (c) 2018 Hans Liljestrand. All rights reserved.
//

#include "llvm/PARTS/PartsIntr.h"
#include "llvm/PARTS/PartsTypeMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;
using namespace llvm::PARTS;

Value *PartsIntr::pac_code_pointer(Function &F, Instruction &I, Value *V, const std::string &name) {
  // Get the intrinsic declaration based on our specific pointer type
  Type *arg_types[] = { V->getType() };
  auto pacia = Intrinsic::getDeclaration(F.getParent(), Intrinsic::pa_pacia, arg_types);

  // Create the arguments for the intrinsic call (i.e., original pointer + modifier/type_id)
  Value *args[] = { V, PartsTypeMetadata::idConstantFromType(F.getContext(), V->getType()) };

  // insert the call
  IRBuilder<> Builder(&I);
  return Builder.CreateCall(pacia, args, name);
}

Value *PartsIntr::pac_code_pointer(Function &F, Instruction &I, Value *V) {
  return pac_code_pointer(F, I, V, "paced_ptr");
}
