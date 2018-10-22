//===----------------------------------------------------------------------===//
//
// Authors: Hans Liljestrand <hans.liljestrand@pm.me>
//          Zaheer Ahmed Gauhar
// Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <llvm/PARTS/PartsTypeMetadata.h>

#include "llvm/PARTS/PartsTypeMetadata.h"

extern "C" {
// A bit ugly, but works...
#include "../PARTS-sha3/include/sha3.h"
}

#define PARTS_USE_SHA3

using namespace llvm;

PartsTypeMetadata::PartsTypeMetadata(type_id_t type_id)
    : m_type_id(type_id) {
  // Do we need to do something?
}

PartsTypeMetadata::PartsTypeMetadata(const MDNode *MDN) {
  assert(MDN != nullptr && "cannot construct from nullptr");
  assert(isPartsTypeMetadataContainer(MDN) && "Constructor must have valid MDNode");

  unsigned i = 1;
  m_type_id = dyn_cast<ConstantAsMetadata>(MDN->getOperand(i++))->getValue()->getUniqueInteger().getLimitedValue(UINT64_MAX);
  m_known = 1 == (dyn_cast<ConstantAsMetadata>(MDN->getOperand(i++))->getValue()->getUniqueInteger().getLimitedValue(1));
  m_pointer = 1 == (dyn_cast<ConstantAsMetadata>(MDN->getOperand(i++))->getValue()->getUniqueInteger().getLimitedValue(1));
  m_data = 1 == (dyn_cast<ConstantAsMetadata>(MDN->getOperand(i++))->getValue()->getUniqueInteger().getLimitedValue(1));
  m_ignored = 1 == (dyn_cast<ConstantAsMetadata>(MDN->getOperand(i++))->getValue()->getUniqueInteger().getLimitedValue(1));
  m_isPACed = 1 == (dyn_cast<ConstantAsMetadata>(MDN->getOperand(i++))->getValue()->getUniqueInteger().getLimitedValue(1));
}

PartsTypeMetadata::~PartsTypeMetadata() {
}

MDNode *PartsTypeMetadata::getMDNode(LLVMContext &C) {

  Metadata* vals[numNodes] = {
      MDString::get(C, MetadataKindString),
      ConstantAsMetadata::get(Constant::getIntegerValue(Type::getInt64Ty(C), APInt(64, m_type_id))),
      ConstantAsMetadata::get(Constant::getIntegerValue(Type::getInt64Ty(C), APInt(1, m_known ? 1 : 0))),
      ConstantAsMetadata::get(Constant::getIntegerValue(Type::getInt64Ty(C), APInt(1, m_pointer ? 1 : 0))),
      ConstantAsMetadata::get(Constant::getIntegerValue(Type::getInt64Ty(C), APInt(1, m_data ? 1 : 0))),
      ConstantAsMetadata::get(Constant::getIntegerValue(Type::getInt64Ty(C), APInt(1, m_ignored ? 1 : 0))),
      ConstantAsMetadata::get(Constant::getIntegerValue(Type::getInt64Ty(C), APInt(1, m_isPACed ? 1 : 0))),
  };
  return MDNode::get(C, vals);
}

void PartsTypeMetadata::attach(LLVMContext &C, Instruction &I) {
  I.setMetadata(MetadataKindString, getMDNode(C));
}

std::string PartsTypeMetadata::toString() {
  return "[ignored:" + std::to_string((isIgnored() ? 1 : 0)) +
         "[PACed:" + std::to_string((isPACed() ? 1 : 0)) +
         "," + (isPointer() ? (isCodePointer() ? "codePointer" : "dataPointer") : "not-a-pointer") +
         ",id:" + std::to_string(getTypeId()) + "]";
}

MDNode *PartsTypeMetadata::retrieveAsMDNode(const Instruction *I) {
  auto MDN = I->getMetadata(MetadataKindString);

  if (isPartsTypeMetadataContainer(MDN)) {
    assert(isa<ConstantAsMetadata>(MDN->getOperand(1)) && "Cannot find PartsTypeMetadata although kind is okay?");
    return MDN;
  }

  //errs() << "Found nothing\n";
  return nullptr;
}

const PartsTypeMetadata_ptr PartsTypeMetadata::retrieve(const MachineInstr &MI)
{
  const auto numOps = MI.getNumOperands();

  for (unsigned i = 0; i < numOps; i++) {
    const auto op = MI.getOperand(i);

    if (op.isMetadata()) {
      if (const PartsTypeMetadata_ptr n = retrieve(op.getMetadata()))
        return n;
    }
  }
  return nullptr;
}

const PartsTypeMetadata_ptr PartsTypeMetadata::retrieve(const MDNode *MDNp)
{
  if (isPartsTypeMetadataContainer(MDNp)) {
    return std::make_shared<PartsTypeMetadata>(MDNp);
  }

  if (MDNp->getNumOperands() == 1) {
    const auto &op = MDNp->getOperand(0);
    if (isa<MDNode>(op))
      return retrieve(dyn_cast<MDNode>(op));
  }

  return nullptr;
}

PartsTypeMetadata_ptr PartsTypeMetadata::get(const type_id_t type_id) {
  return std::make_shared<PartsTypeMetadata>(type_id);
}

PartsTypeMetadata_ptr PartsTypeMetadata::get(const Type *const type)
{
  auto TMD = get(idFromType(type));
  TMD->setIsKnown(true);

  if (TyIsPointer(type)) {
    TMD->setIsPointer(true);
    TMD->setIsCodePointer(TyIsCodePointer(type));
  } else {
    TMD->setIsPointer(false);
  }

  return TMD;
}

PartsTypeMetadata_ptr PartsTypeMetadata::getUnknown()
{
  return std::make_shared<PartsTypeMetadata>(0);
}

PartsTypeMetadata_ptr PartsTypeMetadata::getIgnored()
{
  auto TMD =  std::make_shared<PartsTypeMetadata>(0);
  TMD->setIgnored(true);
  
  return TMD;
}

bool PartsTypeMetadata::isPartsTypeMetadataContainer(const MDNode *const MDN) {
  return (MDN != nullptr && MDN->getNumOperands() == numNodes &&
          isa<MDString>(MDN->getOperand(0)) && isa<ConstantAsMetadata>(MDN->getOperand(1)) &&
          dyn_cast<MDString>(MDN->getOperand(0))->getString() == MetadataKindString);
}


type_id_t PartsTypeMetadata::idFromType(const Type *const type)
{
  if (!TyIsPointer(type))
    return 0;

  type_id_t type_id = 0;
  // Generate a std::string from type
  std::string type_str;
  llvm::raw_string_ostream rso(type_str);
  type->print(rso);

  auto c_string = rso.str().c_str();

#ifdef PARTS_USE_SHA3


  // Prepare SHA3 generation
  mbedtls_sha3_context sha3_context;
  mbedtls_sha3_type_t sha3_type = MBEDTLS_SHA3_256;
  mbedtls_sha3_init(&sha3_context);

  // Prepare input and output variables
  auto *input = reinterpret_cast<const unsigned char*>(c_string);
  auto *output= new unsigned char[32]();

  // Generate hash
  auto result = mbedtls_sha3(input, type_str.length(), sha3_type, output);
  if (result != 0)
    llvm_unreachable("SHA3 hashing failed :(");

  memcpy(&type_id, output, sizeof(type_id_t));

  // FIXME: Support proper PA modifiers (this is a workaround for PartsUtils inability to move larger immediate value)
  //const auto original_type_id = type_id;
  //type_id = type_id % UINT8_MAX;

#else
  if (TyIsCodePointer(type))
    type_id = 7;
  else
    type_id = 3;
#endif
  return type_id;
}

Constant *PartsTypeMetadata::idConstantFromType(LLVMContext &C, const Type *const type) {
  return Constant::getIntegerValue(Type::getInt64Ty(C), APInt(64, idFromType(type)));
}

raw_ostream &operator<<(raw_ostream &stream, const PartsTypeMetadata_ptr pmd) {
  stream << "PartsTypeMetadata(" << pmd->getTypeId();
  return stream;
}

Constant *PartsTypeMetadata::getTypeIdConstant(LLVMContext &C) const {
  return Constant::getIntegerValue(Type::getInt64Ty(C), APInt(64, getTypeId()));
}



