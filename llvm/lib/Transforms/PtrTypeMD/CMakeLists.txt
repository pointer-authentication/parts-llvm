#===----------------------------------------------------------------------===//
#
# Author: Hans Liljestrand <hans.liljestrand@pm.me>
# Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//

# If we don't need RTTI or EH, there's no reason to export anything
# from the hello plugin.
if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/PtrTypeMD.exports)
  endif()
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

add_llvm_loadable_module(LLVMPtrTypeMDPass
  PartsPassCpi.cpp
    PtrTypeMD.cpp
    PauthPacMain.cpp
    PauthMarkGlobals.cpp
    DEPENDS intrinsics_gen
    PLUGIN_TOOL opt
    )



