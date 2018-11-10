#===----------------------------------------------------------------------===//
#
# Author: Hans Liljestrand <hans.liljestrand@pm.me>
# Copyright: Secure Systems Group, Aalto University https://ssg.aalto.fi/
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//

add_llvm_library(Parts
  Parts.cpp
  PartsLog.cpp
  PartsLogStream.cpp
  PartsTypeMetadata.cpp
  PartsEventCount.cpp
  PartsIntr.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/PARTS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Support
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR

  DEPENDS
  #intrinsics_gen
  LLVMCore
  LLVMSupport
  LLVMCodeGen
  PARTSsha3
  #LLVMObject

  LINK_LIBS
  LLVMCore
  LLVMSupport
  LLVMCodeGen
  PARTSsha3
  #LLVMObject
  )

set_property(TARGET Parts PROPERTY LLVM_SYSTEM_LIBS)
