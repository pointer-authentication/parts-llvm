//
// Created by Hans Liljestrand on 15/08/18.
// Copyright (c) 2018 Hans Liljestrand. All rights reserved.
//

#ifndef LLVM_PARTSFASTISEL_H
#define LLVM_PARTSFASTISEL_H

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/FastISel.h"

namespace llvm {

class PartsFastISel;
typedef std::shared_ptr<PartsFastISel> PartsFastISel_ptr;

class PartsFastISel {
  FunctionLoweringInfo &FuncInfo;

protected:
public:

  PartsFastISel() = delete;
  PartsFastISel(FunctionLoweringInfo &FuncInfo);

  void addPartsTypeMetadata(MachineInstrBuilder &MIB, MDNode *partsType);

  void addMetadataToStore(MachineInstrBuilder &MIB, MDNode *partsType);
  void addMetadataToLoad(MachineInstrBuilder &MIB, MDNode *partsType);

  void addMetadataToCall(MachineInstrBuilder &MIB, MDNode *partsType);
  void addMetadataToCall(MachineInstrBuilder &MIB, FastISel::CallLoweringInfo &CLI, unsigned reg);

  static PartsFastISel_ptr get(FunctionLoweringInfo &FuncInfo);

};

} // namespace llvm

#endif //LLVM_PARTSFASTISEL_H
