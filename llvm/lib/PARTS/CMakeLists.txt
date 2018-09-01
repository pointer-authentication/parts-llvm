add_llvm_library(Parts
  Parts.cpp
  PartsLog.cpp
  PartsLogStream.cpp
  PartsTypeMetadata.cpp
  PartsIntr.cpp
  sha3/sha3.c
  sha3/keccak.c

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/PARTS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Support
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR

  DEPENDS
  #intrinsics_gen
  LLVMCore
  LLVMSupport
  #LLVMObject

  LINK_LIBS
  LLVMCore
  LLVMSupport
  #LLVMObject
  )

set_property(TARGET Parts PROPERTY LLVM_SYSTEM_LIBS)
