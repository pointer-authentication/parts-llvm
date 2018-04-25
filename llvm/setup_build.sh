#!/bin/bash

# Install will be done to ${INSTALL_DIR}/llvm${REL_NAME}
# modify as needed
INSTALL_DIR=$HOME/opt/llvm
REL_NAME=-rel60_debug

# Set the compiler+tools to use, or uncomment for default
# build_tool=Ninja
build_tool="Unix Makefiles"
# C_COMPILER=clang
# CXX_COMPILER=clang++
C_COMPILER=gcc
CXX_COMPILER=g++

# Define this to set CMAKE_ASM_COMPILRE (maybe not needed)
asm_compiler=as

# Define this if you need custom swig
# swig_executable=${HOME}/opt/swig/bin/swig

# Set to limit parallel workloads
# PARALLEL_LINK_JOBS=3
# PARALLEL_COMPILE_JOBS=3

# Modify this to change target platforms
BUILD_TARGETS="X86;AArch64"

# Use the gold linker
use_gold=1

# Set to Release for non-debug build
build_type="Debug"

# Set to define LLVM_BINUTILS_INCDIR, NOTE: gcc version
# (not sure anymore what this fixes?)
# binutils_incdir=/usr/lib/gcc/x86_64-linux-gnu/6/plugin/include/

# These are automatically generated based on INSTALL_DIR and REL_NAME
declare -r llvm_root="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r install_prefix=${INSTALL_DIR}/llvm${REL_NAME}
declare -r ocaml_install_path=${INSTALL_DIR}/ocaml${REL_NAME}

args="
    -DCMAKE_INSTALL_PREFIX=${install_prefix}
    -DLLVM_OCAML_INSTALL_PATH=${ocaml_install_path}
    -DCMAKE_BUILD_TYPE=${build_type}
    -DLLVM_TARGETS_TO_BUILD=${BUILD_TARGETS}
    -DLLVM_ENABLE_CXX1Y=On
    -LLVM_DENABLE_LLD=On
    -DLLVM_CCACHE_BUILD=On
    -DLLVM_PARALLEL_LINK_JOBS=${PARALLEL_LINK_JOBS}
    -DLLVM_PARALLEL_COMPILE_JOBS=${PARALLEL_COMPILE_JOBS}
    -DLLVM_BUILD_TESTS=On
    -DLLVM_BUILD_EXAMPLES=Off
    -DLLVM_INCLUDE_EXAMPLES=Off
    -DLLVM_ENABLE_LTO=Off
    -DLLVM_BUILD_DOCS=On
    -DLLVM_ENABLE_DOXYGEN=Off
    -DLLVM_ENABLE_RTTI=On
    -DLLVM_INSTALL_BINUTILS_SYMLINKS=On
    -DLLVM_INSTALL_CCTOOLS_SYMLINKS=On
    "

if [[ -n ${swig_executable} ]]; then
    args="${args}
    -DSWIG_EXECUTABLE=${swig_executable}
    "
fi

if [[ -n ${asm_compiler} ]]; then
    args="${args}
    -DCMAKE_ASM_COMPILER=${asm_compiler}
    "
fi

if [[ -n ${C_COMPILER} ]]; then
    args="${args}
    -DCMAKE_C_COMPILER=${C_COMPILER}
    -DLLDB_TEST_C_COMPILER=${C_COMPILER}
    "
fi

if [[ -n ${CXX_COMPILER} ]]; then
    args="${args}
    -DCMAKE_CXX_COMPILER=${CXX_COMPILER}
    -DLLDB_TEST_CXX_COMPILER=${CXX_COMPILER}
    "
fi

if [[ -n ${binutils_incdir} ]]; then
    args="${args}
    -DLLVM_BINUTILS_INCDIR=${binutils_incdir}
    "
fi

if [[ -n ${use_gold} ]]; then
    args="${args}
    -DLLVM_USE_LINKER=gold
    "
fi

if [[ $1 = args ]]; then
    printf "%s\n" $args
elif [[ $1 = setup ]]; then
    echo "nope"
    set -x
    cmake -G "${build_tool}" $args "${llvm_root}"
else
    echo "$(basename "$0") args|setup"
    echo "  setup   - create cmake build in CWD ($(pwd))"
    echo "  args    - dump cmake args($(pwd))"
fi
