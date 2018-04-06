#!/bin/bash

# Install will be done to ${INSTALL_DIR}/llvm${REL_NAME}
INSTALL_DIR=$HOME/opt/llvm
REL_NAME=-rel60_debug

# Set the compiler+tools to use
BUILD_TOOL=Ninja
C_COMPILER=clang
CXX_COMPILER=clang++

# Set to limit parallel workloads
# PARALLEL_LINK_JOBS=3
# PARALLEL_COMPILE_JOBS=3

BUILD_TARGETS="X86;ARM;AArch64"

declare -r llvm_root="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r install_prefix=${INSTALL_DIR}/llvm${REL_NAME}
declare -r ocaml_install_path=${INSTALL_DIR}/ocaml${REL_NAME}

cmake -G ${BUILD_TOOL} \
    -DCMAKE_INSTALL_PREFIX=${install_prefix} \
    -DLLVM_OCAML_INSTALL_PATH=${ocaml_install_path} \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=${C_COMPILER} \
    -DCMAKE_CXX_COMPILER=${CXX_COMPILER} \
    -DLLDB_TEST_C_COMPILER=${C_COMPILER} \
    -DLLDB_TEST_CXX_COMPILER=${CXX_COMPILER} \
    -DLLVM_TARGETS_TO_BUILD="${BUILD_TARGETS}" \
    -DLLVM_ENABLE_CXX1Y=On \
    -LLVM_DENABLE_LLD=On \
    -DLLVM_CCACHE_BUILD=On \
    -DLLVM_PARALLEL_LINK_JOBS=${PARALLEL_LINK_JOBS} \
    -DLLVM_PARALLEL_COMPILE_JOBS=${PARALLEL_COMPILE_JOBS} \
    -DLLVM_BUILD_TESTS=On \
    -DLLVM_BUILD_EXAMPLES=Off \
    -DLLVM_INCLUDE_EXAMPLES=Off \
    -DLLVM_ENABLE_LTO=Off \
    -DLLVM_BUILD_DOCS=On \
    -DLLVM_ENABLE_DOXYGEN=Off \
    -DLLVM_ENABLE_RTTI=On \
    -DLLVM_USE_LINKER=gold \
    -DLLVM_INSTALL_BINUTILS_SYMLINKS=On \
    -DLLVM_INSTALL_CCTOOLS_SYMLINKS=On \
    -DLLVM_BINUTILS_INCDIR=/usr/lib/gcc/x86_64-linux-gnu/6/plugin/include/ \
    ${llvm_root}
