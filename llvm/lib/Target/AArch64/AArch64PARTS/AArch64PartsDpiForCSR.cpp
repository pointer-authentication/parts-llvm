//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea <carlos.chinea.perez@huawei.com>
// Copyright (C) 2019 Huawei Finland Oy
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// LLVM includes
#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "AArch64RegisterInfo.h"
#include "AArch64InstrInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/LivePhysRegs.h"
#include "llvm/CodeGen/MachineDominanceFrontier.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/RDFCopy.h"
#include "llvm/CodeGen/RDFDeadCode.h"
#include "llvm/CodeGen/RDFGraph.h"
#include "llvm/CodeGen/RDFLiveness.h"
#include "llvm/CodeGen/RDFRegisters.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace rdf;

namespace {

  class AArch64PartsDpiForCSR : public MachineFunctionPass {
  public:
    AArch64PartsDpiForCSR() : MachineFunctionPass(ID) {}

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<MachineDominatorTree>();
      AU.addRequired<MachineDominanceFrontier>();
      AU.setPreservesAll();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    StringRef getPassName() const override {
      return "AArch64Parts RDF optimizations";
    }

    bool runOnMachineFunction(MachineFunction &MF) override;

    MachineFunctionProperties getRequiredProperties() const override {
      return MachineFunctionProperties().set(
          MachineFunctionProperties::Property::NoVRegs);
    }

    static char ID;
  private:
    const TargetInstrInfo *TII = nullptr;
    const TargetRegisterInfo *TRI = nullptr;
    MachineRegisterInfo *MRI = nullptr;

    bool handleInstruction(NodeAddr<StmtNode *> SA, DataFlowGraph &DFG);
    void getMachineInstrLivePhysReg(MachineInstr *MI, LivePhysRegs &LV);
    bool doesCSRDefholdDataPtr(DataFlowGraph &DFG, MCPhysReg CSRDef);
    bool isInstrNodeDataPtr(DataFlowGraph &DFG, NodeAddr<InstrNode *> I);
    void protectCSR(MachineInstr *MI, MCPhysReg CSR);
    void getLiveCSR(LivePhysRegs &LV, SmallVector<MCPhysReg, 16> &CSRset);
    NodeId getCSRDef(const MCPhysReg &CSR,
                                   DataFlowGraph &DFG,
                                   NodeAddr<StmtNode *> SA);
  };
} // end anonymous namespace

char AArch64PartsDpiForCSR::ID = 0;

INITIALIZE_PASS_BEGIN(AArch64PartsDpiForCSR, "aarch64-parts-dpi-csr",
      "AArch64Parts RDF optimizations", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineDominanceFrontier)
INITIALIZE_PASS_END(AArch64PartsDpiForCSR, "aarch64-parts-dpi-csr",
      "AArch64Parts RDF optimizations", false, false)

FunctionPass *llvm::createAArch64PartsDpiForCSR() {
  return new AArch64PartsDpiForCSR();
}

bool AArch64PartsDpiForCSR::runOnMachineFunction(MachineFunction &MF) {
  bool modified = false;

  const auto &MDT = getAnalysis<MachineDominatorTree>();
  const auto &MDF = getAnalysis<MachineDominanceFrontier>();
  const auto &STI = MF.getSubtarget<AArch64Subtarget>();
  TII = STI.getInstrInfo();
  TRI = STI.getRegisterInfo();
  MRI = &MF.getRegInfo();

  TargetOperandInfo TOI(*TII);
  DataFlowGraph DFG(MF, *TII, *TRI, MDT, MDF, TOI);
  DFG.build(BuildOptions::KeepDeadPhis);

  for (NodeAddr<BlockNode *> BA: DFG.getFunc().Addr->members(DFG))
    for (NodeAddr<StmtNode *> SA: BA.Addr->members_if(DFG.IsCode<NodeAttrs::Stmt>, DFG))
      modified = handleInstruction(SA, DFG) || modified;

  return modified;
}

// FIXME: Duplicate code from AArch64InstrInfo.cpp
static bool isUnprotectedDataPtr(MachineInstr &MI)
{
  unsigned Opc = MI.getOpcode();

  switch (Opc) {
    case AArch64::PARTS_DATA_PTR:
    case AArch64::PARTS_AUTDA:
      return true;
      break;
    default:
      break;
  }
  return false;
}

bool AArch64PartsDpiForCSR::handleInstruction(NodeAddr<StmtNode *> SA,
                                              DataFlowGraph &DFG) {
  auto MI = SA.Addr->getCode();

  if (!MI->isCall())
    return false;

  bool modified = false;

  LivePhysRegs LV(*TRI);
  SmallVector<MCPhysReg, 16> CSRset;

  getMachineInstrLivePhysReg(MI, LV);
  getLiveCSR(LV, CSRset);

  for(auto CSR: CSRset) {
    NodeId CSRDef = getCSRDef(CSR, DFG, SA);
    if (CSRDef)
      if (doesCSRDefholdDataPtr(DFG, CSRDef)) {
        protectCSR(MI, CSR);
        modified = true;
      }
  }
  return modified;
}

bool AArch64PartsDpiForCSR::doesCSRDefholdDataPtr(DataFlowGraph &DFG,
                                                  MCPhysReg CSRDef) {
  NodeAddr<DefNode *> DA = DFG.addr<DefNode *>(CSRDef);
  NodeId CSRUse = DA.Addr->getReachedUse();

  while (CSRUse != 0) {
    NodeAddr<UseNode *> UA = DFG.addr<UseNode *>(CSRUse);
    NodeAddr<InstrNode *> I = UA.Addr->getOwner(DFG);
    if (isInstrNodeDataPtr(DFG, I))
      return true;
    CSRUse = UA.Addr->getSibling();
  }

  return false;
}

bool AArch64PartsDpiForCSR::isInstrNodeDataPtr(DataFlowGraph &DFG,
                                               NodeAddr<InstrNode *> I) {
  if (DFG.IsCode<NodeAttrs::Phi>(I))
    return false;

  auto MI = NodeAddr<StmtNode *>(I).Addr->getCode();

  return isUnprotectedDataPtr(*MI);
}

void AArch64PartsDpiForCSR::protectCSR(MachineInstr *MI, MCPhysReg CSR) {
  auto MBB = MI->getParent();

  BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(AArch64::PARTS_PACDA), CSR)
    .addUse(CSR)
    .addUse(AArch64::SP);
  auto InsertPoint = MI->getNextNode();
  if (InsertPoint)
    BuildMI(*MBB, InsertPoint, MI->getDebugLoc(), TII->get(AArch64::PARTS_AUTDA), CSR)
      .addUse(CSR)
      .addUse(AArch64::SP);
  else
    BuildMI(MBB, MI->getDebugLoc(), TII->get(AArch64::PARTS_AUTDA), CSR)
      .addUse(CSR)
      .addUse(AArch64::SP);
}

void AArch64PartsDpiForCSR::getMachineInstrLivePhysReg(MachineInstr *MI,
                                                       LivePhysRegs &LV) {
  auto MBB = MI->getParent();
  LV.addLiveIns(*MBB);

  for (auto MIi = MBB->instr_rbegin(); &*MIi != MI; ++MIi)
    LV.stepBackward(*MIi);

  LV.stepBackward(*MI);
}

void AArch64PartsDpiForCSR::getLiveCSR(LivePhysRegs &LV,
                                       SmallVector<MCPhysReg, 16> &CSRset)
                                        {
  for (const MCPhysReg *CSR = MRI->getCalleeSavedRegs(); CSR && *CSR; ++CSR)
    if (LV.contains(*CSR))
      CSRset.push_back(*CSR);
}

NodeId AArch64PartsDpiForCSR::getCSRDef(const MCPhysReg &CSR,
                                        DataFlowGraph &DFG,
                                        NodeAddr<StmtNode *> SA) {
  Liveness LV(*MRI, DFG);
  auto RefRR = DFG.makeRegRef(CSR, 0);
  auto RA = LV.getNearestAliasedRef(RefRR, SA);

  if (RA.Id == 0)
    return 0;

  if (RA.Addr->getKind() == NodeAttrs::Def)
    return RA.Id;

  assert(RA.Addr->getKind() == NodeAttrs::Use);

  return RA.Addr->getReachingDef();
}
