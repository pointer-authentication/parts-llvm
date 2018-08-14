#include "llvm/IR/PartsTypeMetadata.h"

using namespace llvm;

PartsTypeMetadata::PartsTypeMetadata(Constant *C, type_id_t type_id)
    : ValueAsMetadata(PartsTypeMetadataKind, C),
      m_type_id(type_id)
{
  // Do we need to do something?
}

PartsTypeMetadata *PartsTypeMetadata::get(LLVMContext &C, type_id_t type_id)
{
  auto *type_id_Constant = Constant::getIntegerValue(
      Type::getInt64Ty(C), APInt(64, type_id));

  return new PartsTypeMetadata(type_id_Constant, type_id);
}

PartsTypeMetadata *PartsTypeMetadata::get(LLVMContext &C, const Type *const type)
{
  auto *TMD = get(C, idFromType(type));
  TMD->setIsKnown(true);
  TMD->setIsPointer(TyIsPointer(type));
  TMD->setIsCodePointer(TyIsCodePointer(type));

  return TMD;
}

type_id_t PartsTypeMetadata::idFromType(const Type *const type)
{
  if (TyIsPointer(type))
    return 1;

  if (TyIsCodePointer(type))
    return 7;

  return 3;
}

const PartsTypeMetadata *PartsTypeMetadata::retrieve(MachineInstr &MI)
{
  const auto numOps = MI.getNumOperands();

  for (unsigned i = 0; i < numOps; i++) {
    const auto op = MI.getOperand(i);

    if (op.isMetadata()) {
      if (const PartsTypeMetadata *n = retrieve(op.getMetadata()))
        return n;
    }
  }
  return nullptr;
}

const PartsTypeMetadata *PartsTypeMetadata::retrieve(const MDNode *MDNp)
{
  if (isa<PartsTypeMetadata>(MDNp))
    return dyn_cast<PartsTypeMetadata>(MDNp);

  if (MDNp->getNumOperands() == 1) {
    const auto &op = MDNp->getOperand(0);
    if (isa<MDNode>(op))
      return retrieve(dyn_cast<MDNode>(op));
  }

  return nullptr;
}
