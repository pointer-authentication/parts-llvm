//===----------------------------------------------------------------------===//
//
// Author: Carlos Chinea <carlos.chinea.perez@huawei.com>
//         Hans Liljestrand <hans@liljestrand.dev>
// Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// LLVM includes
#include "AArch64.h"
#include "AArch64PARTS/PartsSpill.h"
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
    bool doesCSRholdDataPtr(DataFlowGraph &DFG,
                            NodeAddr<StmtNode *> SA,
                            MCPhysReg CSR);
    bool doesCSRDefholdDataPtr(DataFlowGraph &DFG, MCPhysReg CSRDef);
    bool isInstrNodeDataPtrDef(DataFlowGraph &DFG, NodeAddr<InstrNode *> I);
    bool isInstrNodeDataPtrUse(DataFlowGraph &DFG, NodeAddr<InstrNode *> I);
    bool protectCSR(MachineInstr *MI, MCPhysReg CSR);
    void insertPartsIntrinsicForCSR(MachineBasicBlock *MBB,
                                     MachineInstr  *MI,
                                     const DebugLoc &DL,
                                     const MCInstrDesc &MCID,
                                     const MCPhysReg CSR);
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
  if (MF.getFunction().hasFnAttribute("no-parts")) return false;
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

  for(auto CSR: CSRset)
    if (doesCSRholdDataPtr(DFG, SA, CSR))
      modified = protectCSR(MI, CSR);

  return modified;
}

bool AArch64PartsDpiForCSR::doesCSRholdDataPtr(DataFlowGraph &DFG,
                                               NodeAddr<StmtNode *> SA,
                                               MCPhysReg CSR) {
    NodeId CSRDef = getCSRDef(CSR, DFG, SA);
    return CSRDef && doesCSRDefholdDataPtr(DFG, CSRDef);
}

bool AArch64PartsDpiForCSR::doesCSRDefholdDataPtr(DataFlowGraph &DFG,
                                                  MCPhysReg CSRDef) {
  NodeAddr<DefNode *> DA = DFG.addr<DefNode *>(CSRDef);

  if (isInstrNodeDataPtrDef(DFG, DA.Addr->getOwner(DFG)))
    return true;

  NodeId CSRUse = DA.Addr->getReachedUse();

  while (CSRUse != 0) {
    NodeAddr<UseNode *> UA = DFG.addr<UseNode *>(CSRUse);
    NodeAddr<InstrNode *> I = UA.Addr->getOwner(DFG);
    if (isInstrNodeDataPtrUse(DFG, I))
      return true;
    CSRUse = UA.Addr->getSibling();
  }

  return false;
}

bool AArch64PartsDpiForCSR::isInstrNodeDataPtrDef(DataFlowGraph &DFG,
                                               NodeAddr<InstrNode *> I) {
  if (DFG.IsCode<NodeAttrs::Phi>(I))
    return false;

  auto MI = NodeAddr<StmtNode *>(I).Addr->getCode();

  return isDataPtrDef(*MI);
}


bool AArch64PartsDpiForCSR::isInstrNodeDataPtrUse(DataFlowGraph &DFG,
                                               NodeAddr<InstrNode *> I) {
  if (DFG.IsCode<NodeAttrs::Phi>(I))
    return false;

  auto MI = NodeAddr<StmtNode *>(I).Addr->getCode();

  return isDataPtrUse(*MI);
}

bool AArch64PartsDpiForCSR::protectCSR(MachineInstr *MI, MCPhysReg CSR) {
  auto MBB = MI->getParent();
  const auto &DL = MI->getDebugLoc();

  insertPartsIntrinsicForCSR(MBB, MI, DL, TII->get(AArch64::PARTS_PACDA), CSR);
  insertPartsIntrinsicForCSR(MBB, MI->getNextNode(), DL,
                             TII->get(AArch64::PARTS_AUTDA), CSR);

  return true;
}

void AArch64PartsDpiForCSR::insertPartsIntrinsicForCSR(MachineBasicBlock *MBB,
                                                       MachineInstr  *MI,
                                                       const DebugLoc &DL,
                                                       const MCInstrDesc &MCID,
                                                       const MCPhysReg CSR) {
  if (MI)
    BuildMI(*MBB, MI, DL, MCID, CSR).addUse(CSR).addUse(AArch64::SP);
  else
    BuildMI(MBB, DL, MCID, CSR).addUse(CSR).addUse(AArch64::SP);
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
