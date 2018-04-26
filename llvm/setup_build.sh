#!/bin/bash

#######################################################################
# Where to install
#######################################################################

# Install will be done to ${install_dir}/llvm${release_name}
install_dir=$HOME/opt/llvm
release_name=-rel60_debug

# Or, you can directly modify install_prefix and ocaml_install_path
declare -r llvm_root="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r install_prefix=${install_dir}/llvm${release_name}
declare -r ocaml_install_path=${install_dir}/ocaml${release_name}

#######################################################################
# Tools and tool configuration
#######################################################################

# Set the compiler+tools to use, or uncomment for default
# build_tool=Ninja
build_tool="Unix Makefiles"
C_COMPILER=clang
CXX_COMPILER=clang++
# C_COMPILER=gcc
# CXX_COMPILER=g++
enable_cache=On # (On/Off)

# Define this to set CMAKE_ASM_COMPILRE (maybe not needed)
asm_compiler=as

# Set to limit parallel workloads (ninja might otherwise overdo it)
# PARALLEL_LINK_JOBS=3
# PARALLEL_COMPILE_JOBS=3

#######################################################################
# What to build?
#######################################################################

# Set to Release for non-debug build
build_type="Debug"

# Modify this to change target platforms
BUILD_TARGETS="X86;AArch64"

# Using shared libs should speed up intermediate development.
build_shared_libs=On # (default Off)

# Disabling the tools requires manual clang (or other tool) compilation
# but speeds up the compilation considerably.
build_tools=Off # (default On)

build_tests=Off # (default Off)
build_docs=Off # (default Off)
build_examples=Off # (default Off)
include_examples=Off # (default Off)

#######################################################################
# Potential fixes for missing/custom stuff
#######################################################################

# Define this if you need custom swig
# swig_executable=${HOME}/opt/swig/bin/swig

# Set to define LLVM_BINUTILS_INCDIR, NOTE: gcc version
# (not sure anymore what this fixes?)
# binutils_incdir=/usr/lib/gcc/x86_64-linux-gnu/6/plugin/include/

#######################################################################
# Tweaks/Fixes?
#######################################################################

# Use the gold linker (We probably want this for LTO)
use_linker=gold

# Link time optimization will probably need gold linker
enable_lto=Off

# Doxygen (this seems incrediby slow, maybe leave off) 
enable_doxygen=Off

# Enable Runtime Type Information (default off)
enable_rtti=Off

# Use C++11/C++14/etc features
enable_cxx1y=Off

#######################################################################
#######################################################################
#######################################################################
# Code blow, should not need editing
#######################################################################

args="
    -DCMAKE_INSTALL_PREFIX=${install_prefix}
    -DLLVM_OCAML_INSTALL_PATH=${ocaml_install_path}
    -DCMAKE_BUILD_TYPE=${build_type}
    -DLLVM_TARGETS_TO_BUILD=${BUILD_TARGETS}
    -DLLVM_PARALLEL_LINK_JOBS=${PARALLEL_LINK_JOBS}
    -DLLVM_PARALLEL_COMPILE_JOBS=${PARALLEL_COMPILE_JOBS}
    -DLLVM_INSTALL_BINUTILS_SYMLINKS=On
    -DLLVM_INSTALL_CCTOOLS_SYMLINKS=On
    -DLLVM_CCACHE_BUILD=${enable_cache}
    -DLLVM_BUILD_TOOLS=${build_tools}
    -DLLVM_BUILD_TESTS=${build_tests}
    -DLLVM_BUILD_EXAMPLES=${build_examples}
    -DLLVM_BUILD_DOCS=${build_docs}
    -DLLVM_INCLUDE_EXAMPLES=${include_examples}
    -DLLVM_ENABLE_CXX1Y=${enable_cxx1y}
    -DLLVM_ENABLE_LTO=${enable_lto}
    -DLLVM_ENABLE_DOXYGEN=${enable_doxygen}
    -DLLVM_ENABLE_RTTI=${enable_rtti}
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

if [[ -n ${use_linker} ]]; then
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
