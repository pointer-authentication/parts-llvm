# If we don't need RTTI or EH, there's no reason to export anything
# from the hello plugin.
if( NOT LLVM_REQUIRES_RTTI )
  if( NOT LLVM_REQUIRES_EH )
    set(LLVM_EXPORTED_SYMBOL_FILE ${CMAKE_CURRENT_SOURCE_DIR}/TestPass.exports)
  endif()
endif()

if(WIN32 OR CYGWIN)
  set(LLVM_LINK_COMPONENTS Core Support)
endif()

add_llvm_loadable_module(LLVMTestPass
    TestPass.cpp
    PauthPacMain.cpp
    PauthIntrinsics.cpp
    PauthMarkGlobals.cpp
	sha3.c
	keccak.c
    DEPENDS intrinsics_gen
    PLUGIN_TOOL opt
    )



