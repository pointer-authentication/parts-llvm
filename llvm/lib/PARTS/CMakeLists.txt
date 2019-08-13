#===----------------------------------------------------------------------===//
#
# Author: Hans Liljestrand <hans@liljestrand.dev>
#         Carlos Chinea <carlos.chinea.perez@huawei.com>
# Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
# Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//

add_llvm_library(Parts
  Parts.cpp
  PartsOptPass.cpp
  PartsOptRasPass.cpp
  PartsOptBuiltinsPass.cpp
  PartsOptCpiPass.cpp
  PartsOptDataPointerArgsPass.cpp
  PartsOptDpiPass.cpp
  PartsOptGlobalsPass.cpp
  PartsOptMainArgsPass.cpp
  PartsEventCount.cpp
  PartsIntr.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/PARTS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Support
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/IR

  DEPENDS
  intrinsics_gen
  LLVMCore
  LLVMSupport
  LLVMTransformUtils
  LLVMCodeGen
  PARTSsha3

  LINK_LIBS
  LLVMCore
  LLVMSupport
  LLVMTransformUtils
  LLVMCodeGen
  PARTSsha3
  )

set_property(TARGET Parts PROPERTY LLVM_SYSTEM_LIBS)
