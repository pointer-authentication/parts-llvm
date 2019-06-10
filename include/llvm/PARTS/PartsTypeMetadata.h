//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans@liljestrand.dev>
// Copyright (c) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_PARTSTYPEMETADATA_H
#define LLVM_IR_PARTSTYPEMETADATA_H

#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/IR/Metadata.h"

namespace llvm {

namespace PARTS {

typedef uint64_t type_id_t;

}

using namespace PARTS;

class PartsTypeMetadata;

typedef std::shared_ptr<PartsTypeMetadata> PartsTypeMetadata_ptr;

/*! A encapsulating PARTS type_id and related metadata. */
/*! This class encapsulates type_id information and related metadata, in particular, it also
 * keeps track of whether type_id is known, unknown, ignored, and whether the type_id corresponds
 * to a pointer of data or code type. The class also includes utility functions for embedding and retrieving the data
 * into an MDNode, this allows easy attaching and retrieval from various LLVM data-structures.
 */
class PartsTypeMetadata {
  type_id_t m_type_id = 0;
  bool m_known = false;
  bool m_pointer = false;
  bool m_data = false;
  bool m_ignored = false;
  bool m_isPACed = false;

  static constexpr const int numNodes = 7;

  static constexpr auto MetadataKindString = "PartsTypeMetadata";

  PartsTypeMetadata() = delete;

protected:
public:
  /*! Constructor using type_id */
  explicit PartsTypeMetadata(type_id_t type_id);
  /*! Constructor used to restore PartsTypeMetadata from MDNode */
  explicit PartsTypeMetadata(const MDNode *MDN);

  ~PartsTypeMetadata();

  /*! Get a (new) MDNode containing necessary information to re-create this object */
  MDNode *getMDNode(LLVMContext &C);

  /*! Get type_id of associated LLVM Type */
  inline type_id_t getTypeId() const { return m_type_id; }
  Constant *getTypeIdConstant(LLVMContext &C) const;

  /*! Return true if the associated Value should be ignored */
  inline bool isIgnored() const { return m_ignored; }
  /*! Return true if the associated Value is already PACed */
  inline bool isPACed() const { return m_isPACed; }
  /*! Return true if type is known */
  inline bool isKnown() const { return m_known; }
  /*! Return true if type is a pointer */
  inline bool isPointer() const { return m_pointer; }
  /*! Return true if Type is a data pointer */
  inline bool isDataPointer() const { return m_pointer && m_data; }
  /*! Return true if Type is a code pointer */
  inline bool isCodePointer() const { return m_pointer && !m_data; }

  inline void setIgnored(bool ignored);
  inline void setIsPACed(bool isPACed);
  inline void setIsKnown(bool known);
  inline void setIsPointer(bool pointer);
  inline void setIsDataPointer(bool data);
  inline void setIsCodePointer(bool code);

  void attach(LLVMContext &C, Instruction &I);

  std::string toString();

  static PartsTypeMetadata_ptr get(type_id_t type_id);
  static PartsTypeMetadata_ptr get(const Type *type);
  static PartsTypeMetadata_ptr getUnknown();
  static PartsTypeMetadata_ptr getIgnored();
  static const PartsTypeMetadata_ptr retrieve(const MachineInstr &MI);
  static const PartsTypeMetadata_ptr retrieve(const MDNode *MDNp);

  static bool isPartsTypeMetadataContainer(const MDNode *const MDN);

  static MDNode *retrieveAsMDNode(const Instruction *I);

  inline static bool TyIsCodePointer(const Type *const type);
  inline static bool TyIsPointer(const Type *const type);

  static type_id_t idFromType(const Type *const type);
  static Constant *idConstantFromType(LLVMContext &context, const Type *const type);

  friend raw_ostream &operator<<(raw_ostream &stream, const PartsTypeMetadata_ptr);
};

inline bool PartsTypeMetadata::TyIsCodePointer(const Type *const type)
{
  return type->isPointerTy() && type->getPointerElementType()->isFunctionTy();
}

inline bool PartsTypeMetadata::TyIsPointer(const Type *const type)
{
  return type->isPointerTy();
}

inline void PartsTypeMetadata::setIgnored(bool ignored)
{
  m_ignored = ignored;
}

inline void PartsTypeMetadata::setIsPACed(bool isPACed) {
  m_isPACed = isPACed;
}

inline void PartsTypeMetadata::setIsKnown(bool known)
{
  m_known = known;
}

inline void PartsTypeMetadata::setIsPointer(bool pointer)
{
  m_known = true;
  m_pointer = pointer;
}

inline void PartsTypeMetadata::setIsDataPointer(bool data)
{
  m_known = true;
  m_data = data;
}
inline void PartsTypeMetadata::setIsCodePointer(bool code)
{
  m_known = true;
  m_data = !code;
}

} // namespace llvm

#endif // LLVM_IR_PARTSTYPEMETADATA_H
