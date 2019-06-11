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

  class AArch64PartsRDFOpt : public MachineFunctionPass {
  public:
    AArch64PartsRDFOpt() : MachineFunctionPass(ID) {}

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
    const AArch64InstrInfo *TII = nullptr;
    MachineRegisterInfo *MRI;
  };


  struct AArch64PartsDCE : public DeadCodeElimination {
    AArch64PartsDCE(DataFlowGraph &G, MachineRegisterInfo &MRI)
      : DeadCodeElimination(G, MRI) {}

    bool run();
  };
} // end anonymous namespace

char AArch64PartsRDFOpt::ID = 0;

INITIALIZE_PASS_BEGIN(AArch64PartsRDFOpt, "aarch64-parts-rdf-opt",
      "AArch64Parts RDF optimizations", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTree)
INITIALIZE_PASS_DEPENDENCY(MachineDominanceFrontier)
INITIALIZE_PASS_END(AArch64PartsRDFOpt, "aarch64-parts-rdf-opt",
      "AArch64Parts RDF optimizations", false, false)

FunctionPass *llvm::createAArch64PartsRDFOpt() {
  return new AArch64PartsRDFOpt();
}

bool AArch64PartsRDFOpt::runOnMachineFunction(MachineFunction &MF) {
  bool modified = false;

  const auto &MDT = getAnalysis<MachineDominatorTree>();
  const auto &MDF = getAnalysis<MachineDominanceFrontier>();
  MRI = &MF.getRegInfo();
  const auto &TRI = *MF.getSubtarget<AArch64Subtarget>().getRegisterInfo();
  TII = MF.getSubtarget<AArch64Subtarget>().getInstrInfo();

  TargetOperandInfo TOI(*TII);
  DataFlowGraph G(MF, *TII, TRI, MDT, MDF, TOI);

  G.build(BuildOptions::KeepDeadPhis);
  AArch64PartsDCE DCE(G, *MRI);

  DCE.trace(true);
  modified = DCE.run();

  if (modified) {
    dbgs() << "Starting liveness recomputation on: " << MF.getName() << '\n';
    Liveness LV(*MRI, G);
    LV.trace(true);
    LV.computeLiveIns();
    LV.resetLiveIns();
    LV.resetKills();
  }

  return modified;
}

bool AArch64PartsDCE::run(void)
{
  if (!collect())
    return false;

  return erase(getDeadInstrs());
}
