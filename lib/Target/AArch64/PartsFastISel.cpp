//
// Created by Hans Liljestrand on 15/08/18.
// Copyright (c) 2018 Hans Liljestrand. All rights reserved.
//

#include "PartsFastISel.h"
#include "PointerAuthentication.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/PartsTypeMetadata.h"

using namespace llvm;

PartsFastISel::PartsFastISel(FunctionLoweringInfo &FuncInfo)
    : FuncInfo(FuncInfo) {}

PartsFastISel_ptr PartsFastISel::get(FunctionLoweringInfo &FuncInfo) {
  return std::make_shared<PartsFastISel>(FuncInfo);
}

void PartsFastISel::addPartsTypeMetadata(MachineInstrBuilder &MIB, MDNode *partsType) {
  assert(partsType != nullptr && "Expected a non-null MDNode pointer!");
  assert(isa<MDNode>(partsType) && "Expected an MDNode");
  partsType->dump();
  assert(PartsTypeMetadata::isPartsTypeMetadataContainer(partsType) && "Not a PartsTypeMetadata MDNode!");
  
  MIB.addMetadata(partsType);
}

void PartsFastISel::addMetadataToStore(MachineInstrBuilder &MIB, MDNode *partsType) {
#ifdef ENABLE_PAUTH_SLLOW
  if (partsType == nullptr) {
    DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** no metadata when emitting store\n");
    return;
  }
  DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** moving metadata to emitted store\n");

  addPartsTypeMetadata(MIB, partsType);
#endif
}


void PartsFastISel::addMetadataToLoad(MachineInstrBuilder &MIB, MDNode *partsType) {
#ifdef ENABLE_PAUTH_SLLOW
  if (partsType == nullptr) {
    DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** no metadata when emitting LDR\n");
    return;
  }
  DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** moving metadata to emitted LDR\n");

  addPartsTypeMetadata(MIB, partsType);
#endif
}

void PartsFastISel::addMetadataToCall(MachineInstrBuilder &MIB, MDNode *partsType) {
#ifdef ENABLE_PAUTH_SLLOW
  if (partsType == nullptr) {
    DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** no metadata when emitting call\n");
    return;
  }
  DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** moving metadata to emitted call\n");

  addPartsTypeMetadata(MIB, partsType);
#endif /* ENABLE_PAUTH_SLLOW */
}

void PartsFastISel::addMetadataToCall(MachineInstrBuilder &MIB, FastISel::CallLoweringInfo &CLI, unsigned reg)
{
#ifdef ENABLE_PAUTH_SLLOW
  // prep for pauth instrumentation by transferring type_id info to emitted BLR

  PartsTypeMetadata_ptr partsType = nullptr;

  auto &C = FuncInfo.Fn->getContext();
  const Value *Callee = CLI.Callee;

  if (reg) {
    DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** preparing metadata to emitted branch instruction\n");
    partsType = PartsTypeMetadata::get(Callee->getType());
  } else {
    DEBUG_PA_LOW(FuncInfo.Fn, errs() << "\t\t\t*** setting ignore metadata to emitted branch instruction\n");
    partsType = PartsTypeMetadata::getIgnored();
  }

  addPartsTypeMetadata(MIB, partsType->getMDNode(C));
#endif /* ENABLE_PAUTH_SLLOW */
}
