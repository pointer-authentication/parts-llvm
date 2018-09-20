//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/IR/IRBuilder.h>
#include <llvm/PARTS/PartsIntr.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/PARTS/Parts.h"
#include "llvm/PARTS/PartsLog.h"
#include "llvm/PARTS/PartsTypeMetadata.h"

using namespace llvm;

#define DEBUG_TYPE "PauthOptPauthMarkGlobals"
#define TAG KYEL DEBUG_TYPE ": "

#define DEBUG_DO(x) do { \
  x;\
  errs() << KNRM; \
} while(0);

#ifdef DISABLE_PA_DEBUG
#undef DEBUG_DO
#define DEBUG_DO(x)
#endif

namespace {

struct PauthMarkGlobals: public FunctionPass {
  static char ID; // Pass identification, replacement for typeid

  PartsLog_ptr log;

  std::list<PARTS::type_id_t> data_type_ids = std::list<PARTS::type_id_t>(0);
  std::list<PARTS::type_id_t> code_type_ids = std::list<PARTS::type_id_t>(0);
  int marked_data_pointers = 0;
  int marked_code_pointers = 0;
  int fixed_dp = 0;
  int fixed_cp = 0;
  bool need_fix_globals_call = false;

  IRBuilder<> *builder;

  Function *funcFixGlobals = nullptr;

  PauthMarkGlobals() :
      FunctionPass(ID),
      log(PartsLog::getLogger(DEBUG_TYPE))
  {
    DEBUG_PA(log->enable());
  }

  bool doInitialization(Module &M) override;
  bool runOnFunction(Function &M) override;

  bool handleGlobal(Module &M, GlobalVariable &GV);

private:
  void writeTypeIds(Module &M, std::list<PARTS::type_id_t> &type_ids, const char *sectionName);

};

} // anonymous namespace

char PauthMarkGlobals::ID = 0;
static RegisterPass<PauthMarkGlobals> X("pauth-markglobals", "PAC argv for main call");

bool PauthMarkGlobals::doInitialization(Module &M) {
  if ( !(PARTS::useFeCfi() || PARTS::useDpi())) // We don't need to do anything unless we use PI
    return false;

  auto &C = M.getContext();

  auto result = Type::getVoidTy(C);
  FunctionType* signature = FunctionType::get(result, false);
  funcFixGlobals = Function::Create(signature, Function::PrivateLinkage, "__pauth_pac_globals", &M);

  auto BB = BasicBlock::Create(M.getContext(), "entry", funcFixGlobals);
  IRBuilder<> localBuilder(BB);
  builder = &localBuilder;
  for (auto GI = M.global_begin(); GI != M.global_end(); GI++) {
    handleGlobal(M, *GI);
  }
  builder->CreateRetVoid();
  builder = nullptr;

  // FIXME: There's something wrong with this reporting!?!
  if (PARTS::useFeCfi()) {
    log->inc(DEBUG_TYPE ".CodePointersFixed") << "\"fixed\" " << fixed_cp << " code pointers for PACing\n";
    log->inc(DEBUG_TYPE ".CodePointersMarked") << "annotating " << marked_code_pointers << " code pointers for PACing\n";
    errs() << fixed_cp << " and " << marked_code_pointers << "\n";
    writeTypeIds(M, code_type_ids, ".code_type_id");
  }

  if (PARTS::useDpi()) {
    log->inc(DEBUG_TYPE ".DataPointersFixed") << "\"fixed\" " << fixed_dp << " data pointers for PACing\n";
    log->inc(DEBUG_TYPE ".DataPointersMarked") << "annotating " << marked_data_pointers << " data pointers for PACing\n";
    writeTypeIds(M, data_type_ids, ".data_type_id");
  }

  need_fix_globals_call = (marked_code_pointers+marked_code_pointers+fixed_cp+fixed_dp) > 0;
  return need_fix_globals_call;
}

bool PauthMarkGlobals::runOnFunction(Function &F) {
  if (!(need_fix_globals_call && PARTS::useAny() && F.getName().equals("main")))
    return false;

  assert(F.getName().equals("main"));

  auto &B = F.getEntryBlock();
  auto &I = *B.begin();

  IRBuilder<> Builder(&I);
  Builder.CreateCall(funcFixGlobals);

  DEBUG_PA(log->info() << "Adding call to __pauth_pac_globals\n");
  return true;
}

bool PauthMarkGlobals::handleGlobal(Module &M, GlobalVariable &GV) {
  auto &C = M.getContext();

  log->info() << "inspecting " << GV << "\n";

  if (GV.getNumOperands() == 0) {
    log->info() << "skipping empty\n";
    return false;
  }

  auto O = GV.getOperand(0);
  auto Ty = O->getType();

  if (Ty->isArrayTy() && Ty->getArrayElementType()->isPointerTy()) {
    assert(isa<User>(O));

    auto arrayType = dyn_cast<ArrayType>(O->getType());
    auto elementType = arrayType->getElementType();

    auto PTMD = PartsTypeMetadata::get(elementType);

    if ((PARTS::useDpi() && PTMD->isDataPointer()) ||
        (PARTS::useFeCfi() && PTMD->isCodePointer())) {
      // Only PAC if feature enabled

      for (auto i = 0U; i < dyn_cast<User>(O)->getNumOperands(); i++) {
        auto elPtr = builder->CreateGEP(&GV, {
            ConstantInt::get(Type::getInt64Ty(C), 0),
            ConstantInt::get(Type::getInt64Ty(C), i),
        });

        auto loaded = builder->CreateLoad(elPtr);
        auto paced = PartsIntr::pac_pointer(builder, M, loaded);

        if (PTMD->isCodePointer()) {
          fixed_cp++;
        } else {
          fixed_dp++;
        }

        builder->CreateStore(paced, elPtr);
      }
    }

    if (PARTS::useDpi()) {
      marked_data_pointers++;
      GV.setSection(".data_pauth");
      data_type_ids.push_back(PartsTypeMetadata::idFromType(Ty));
    }
    return true;
  }

  if (Ty->isPointerTy()) {
    auto type_id = PartsTypeMetadata::idFromType(Ty);

    if (PartsTypeMetadata::TyIsCodePointer(Ty)) {
      if (PARTS::useFeCfi()) {
        marked_code_pointers++;
        GV.setSection(".code_pauth");
        code_type_ids.push_back(type_id);
      }
    } else {
      if (PARTS::useDpi()) {
        marked_data_pointers++;
        GV.setSection(".data_pauth");
        data_type_ids.push_back(type_id);
      }
    }
    return true;
  }

  log->info() << "skipping\n";
  return false;
}

void PauthMarkGlobals::writeTypeIds(Module &M, std::list<PARTS::type_id_t> &type_ids, const char *sectionName)
{
  for (auto type_id : type_ids) {
    ConstantInt* type_id_Constant = ConstantInt::get(Type::getInt64Ty(M.getContext()), type_id);

    GlobalVariable *g = new GlobalVariable(M, Type::getInt64Ty(M.getContext()), true, GlobalValue::PrivateLinkage, type_id_Constant);
    g->setExternallyInitialized(false);
    g->setSection(sectionName);
  }
}
