/**
 *
 */

//#include "llvm/IR/Metadata.h"

namespace llvm {

namespace PARTS {

//typedef uint64_t type_id_t;
typedef int type_id_t;

class PartsTypeMD {
  type_id_t m_TypeId = 0;
  bool m_known = true;
  bool m_pointer = false;
  bool m_data = false;
  bool m_ignored = false;

  //PartsTypeMD() = delete;
  //PartsTypeMD(MDTuple);

protected:
public:

  inline type_id_t getTypeId() { return m_TypeId; }
  inline bool isKnown() { return m_known; }
  inline bool isPointer() { return m_pointer; }
  inline bool isIgnored() { return m_ignored; }
  inline bool isDataPointer() { return m_pointer && m_data; }
  inline bool isCodePointer() { return m_pointer && !m_data; }

  //static PartsTypeMD *get(LLVMContext &C, type_id_t typeId);
};

} // namespace PARTS

} // namespace llvm
