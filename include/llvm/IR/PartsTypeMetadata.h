/**
 *
 */

#include "../CodeGen/MachineInstr.h"
#include "Metadata.h"

namespace llvm {

typedef uint64_t type_id_t;

class PartsTypeMetadata : public ValueAsMetadata {
  type_id_t m_type_id = 0;
  bool m_known = false;
  bool m_pointer = false;
  bool m_data = false;
  bool m_ignored = false;

  PartsTypeMetadata() = delete;
  PartsTypeMetadata(Constant *C, type_id_t type_id);

  inline static bool TyIsCodePointer(const Type *const type);
  inline static bool TyIsPointer(const Type *const type);

protected:
public:

  inline type_id_t getTypeId() { return m_type_id; }

  inline bool isIgnored() { return m_ignored; }
  inline bool isKnown() { return m_known; }
  inline bool isPointer() { return m_pointer; }
  inline bool isDataPointer() { return m_pointer && m_data; }
  inline bool isCodePointer() { return m_pointer && !m_data; }

  inline void setIgnore(bool ignored);
  inline void setIsKnown(bool known);
  inline void setIsPointer(bool pointer);
  inline void setIsDataPointer(bool data);
  inline void setIsCodePointer(bool code);

  static PartsTypeMetadata *get(LLVMContext &C, type_id_t type_id);
  static PartsTypeMetadata *get(LLVMContext &C, const Type *type);

  static const PartsTypeMetadata *retrieve(MachineInstr &MI);
  static const PartsTypeMetadata *retrieve(const MDNode *MDNp);

  static type_id_t idFromType(const Type *const type);
};

inline bool PartsTypeMetadata::TyIsCodePointer(const Type *const type)
{
  return type->getPointerElementType()->isFunctionTy();
}

inline bool PartsTypeMetadata::TyIsPointer(const Type *const type)
{
  return type->isPointerTy();
}

inline void PartsTypeMetadata::setIgnore(bool ignored)
{
  m_ignored = ignored;
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
