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
    bool handleInstruction(NodeAddr<StmtNode *> SA, DataFlowGraph &DFG);
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
  const auto &TRI = *STI.getRegisterInfo();
  const auto &TII = *STI.getInstrInfo();

  TargetOperandInfo TOI(TII);
  DataFlowGraph DFG(MF, TII, TRI, MDT, MDF, TOI);
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

  return false;
}
