add_llvm_library(Parts
  PartsLog.cpp
  PartsLogStream.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/PARTS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Support
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR

  DEPENDS
  intrinsics_gen
  )

set_property(TARGET Parts PROPERTY LLVM_SYSTEM_LIBS)
