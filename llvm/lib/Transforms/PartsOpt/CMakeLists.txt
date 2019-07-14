#===----------------------------------------------------------------------===//
#
# Author: Hans Liljestrand <hans.liljestrand@pm.me>
# Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

add_llvm_library(PartsOpt MODULE
  PartsOptCpiPass.cpp
  PartsOptDpiPass.cpp
  PartsOptMainArgsPass.cpp
  PartsOptGlobalsPass.cpp
  PartsOptDataPointerArgsPass.cpp
  DEPENDS intrinsics_gen
  PLUGIN_TOOL opt
)



