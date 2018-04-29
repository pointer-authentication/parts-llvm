#!/bin/bash

declare -r llvm_root="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
declare -r scriptname=$(basename "$0")
declare -r default_system_llvm="5.0"
declare -r default_system_gcc="5"
declare -r default_build_tool="Unix Makefiles"
declare -r usage="
${scriptname} COMMAND

Helper script to setup LLVM repository and cmake build.

COMMANDs:
    pkgs    - check dependencies
    cmake   - create cmake build in CWD ($(pwd))
    args    - dump cmake args
    repos   - install repositories

The script can be further customized with the environment variables:
    system_llvm=x.0   - system LLVM version to use (default: ${default_system_llvm})
    system_gcc=x      - system GCC version to use (default: ${default_system_gcc})
    build_tool=name   - the generator cmake should use (default: ${default_build_tool})

For further customization, please edit the script directly...
"

# Version (git branc) of LLVM
# Note that the main LLVM repository should be based on same branch. The pa-*
# branches are at the time of this writing all based on release_60.
declare -r llvm_release=release_60

# Set some variables used to check and setup system configuration. Defaults are
# okay for Ubuntu 16.04, you can call the script with these variables set in to
# avoid chaning this file directly.
[[ -n ${system_llvm} ]] || system_llvm=${default_system_llvm}
[[ -n ${system_gcc} ]] || system_gcc=${default_system_gcc}

# Which build tool to use (Ninja should be faster, but had some issues on Ubuntu
# 16.04))
[[ -n ${build_tool} ]] || build_tool=${default_build_tool}

# Projects and tools to include when initializing the LLVM sub-repos with
# `$scriptname repos`. The format is:
# submodule[SUBDIRECTORY_TO_CLONE_INTO]=GIT_REPOSITORY
declare -Ag submodules
submodules["tools/clang"]="https://github.com/llvm-mirror/clang.git"
submodules["tools/lld"]="https://github.com/llvm-mirror/lld.git"
submodules["tools/clang/extra"]="https://github.com/llvm-mirror/clang-tools-extra.git"
submodules["tools/lldb"]="https://github.com/llvm-mirror/lldb.git"

#######################################################################
# Package dependencies
#######################################################################

# Modify this list accordingly if you change some other stuff
declare -a pkg_dependencies=(
git cmake ccache python-dev libncurses5-dev swig libedit-dev
gcc-${system_gcc}-plugin-dev
clang-${system_llvm} libclang-${system_llvm}-dev lld-${system_llvm}
)

#######################################################################
# Where to install
#######################################################################

# Install will be done to ${install_dir}/llvm${release_name}
install_dir=$HOME/opt/llvm
release_name=-rel60_debug


#######################################################################
# Tools and tool configuration
#######################################################################

llvm_dir=/usr/lib/llvm-${system_llvm}

# Set the compiler+tools to use, or uncomment for default
# build_tool=Ninja

C_COMPILER=${llvm_dir}/bin/clang
CXX_COMPILER=${llvm_dir}/bin/clang++
enable_cache=On # (On/Off)

# Use the gold linker (We probably want this for LTO)
use_linker=${llvm_dir}/bin/ld.lld
# use_linker=gold

# Define this to set CMAKE_ASM_COMPILRE (maybe not needed)
# asm_compiler=as

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

build_tools=On # (default On)

build_tests=On # (default Off)
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

# Link time optimization will probably need gold linker
enable_lto=On

# Doxygen (this seems incrediby slow, maybe leave off) 
enable_doxygen=Off

# Enable Runtime Type Information (default off)
enable_rtti=Off

# Use C++11/C++14/etc features
enable_cxx1y=On

#######################################################################
#######################################################################
#######################################################################
# Code blow, should not need editing
#######################################################################

declare -r install_prefix=${install_dir}/llvm${release_name}
declare -r ocaml_install_path=${install_dir}/ocaml${release_name}

is_package_installed() {
    local pkg="$1"
    local log="${scriptname}:${FUNCNAME[0]}"

    [[ -n "${pkg}" ]] || (>&2 echo "${log}: missing argument" && return 1)

    if command -v dpkg >/dev/null 2>&1; then
        if  dpkg -s "${pkg}" > /dev/null 2>&1; then
            return 0;
        fi
        return 1
    fi

    >&2 echo "checks.sh:${FUNCNAME[0]}: cannot find dpkg"
    return 1
}

check_packages() {
    local missing=""

    for pkg in "${pkg_dependencies[@]}"; do
        if ! is_package_installed "${pkg}"; then
            missing="${missing} ${pkg}"
        fi
    done

    if [[ -n ${missing} ]]; then
        echo "missing packages: ${missing}"
        return 1
    fi
    return 0
}

echo_cmake_args() {
    echo -DCMAKE_INSTALL_PREFIX=${install_prefix}
    echo -DLLVM_OCAML_INSTALL_PATH=${ocaml_install_path}
    echo -DCMAKE_BUILD_TYPE=${build_type}
    echo -DBUILD_SHARED_LIBS=${build_shared_libs}
    echo -DLLVM_TARGETS_TO_BUILD=${BUILD_TARGETS}
    echo -DLLVM_PARALLEL_LINK_JOBS=${PARALLEL_LINK_JOBS}
    echo -DLLVM_PARALLEL_COMPILE_JOBS=${PARALLEL_COMPILE_JOBS}
    echo -DLLVM_INSTALL_BINUTILS_SYMLINKS=On
    echo -DLLVM_INSTALL_CCTOOLS_SYMLINKS=On
    echo -DLLVM_CCACHE_BUILD=${enable_cache}
    echo -DLLVM_BUILD_TOOLS=${build_tools}
    echo -DLLVM_BUILD_TESTS=${build_tests}
    echo -DLLVM_BUILD_EXAMPLES=${build_examples}
    echo -DLLVM_BUILD_DOCS=${build_docs}
    echo -DLLVM_INCLUDE_EXAMPLES=${include_examples}
    echo -DLLVM_ENABLE_CXX1Y=${enable_cxx1y}
    echo -DLLVM_ENABLE_LTO=${enable_lto}
    echo -DLLVM_ENABLE_DOXYGEN=${enable_doxygen}
    echo -DLLVM_ENABLE_RTTI=${enable_rtti}

    if [[ -n ${swig_executable} ]]; then
        echo -DSWIG_EXECUTABLE=${swig_executable}
    fi

    if [[ -n ${asm_compiler} ]]; then
        echo -DCMAKE_ASM_COMPILER=${asm_compiler}
    fi

    if [[ -n ${C_COMPILER} ]]; then
        echo -DCMAKE_C_COMPILER=${C_COMPILER}
        # echo -DLLDB_TEST_USE_CUSTOM_C_COMPILER=On
        echo -DLLDB_TEST_C_COMPILER=${C_COMPILER}
    fi

    if [[ -n ${CXX_COMPILER} ]]; then
        echo -DCMAKE_CXX_COMPILER=${CXX_COMPILER}
        # -DLLDB_TEST_USE_CUSTOM_CXX_COMPILER=On
        echo -DLLDB_TEST_CXX_COMPILER=${CXX_COMPILER}
    fi

    if [[ -n ${binutils_incdir} ]]; then
        echo -DLLVM_BINUTILS_INCDIR=${binutils_incdir}
    fi

    if [[ -n ${use_linker} ]]; then
        echo -DLLVM_USE_LINKER=${use_linker}
    fi
}

add_submodule() {
    local module=$1
    local path=${llvm_root}/$module
    local cont_dir=$(dirname ${path})
    local repo_dir=$(basename ${path})
    local repo=${submodules[$module]}

    if [[ -e ${path} ]]; then
        if [[ ! -e ${path}/.git ]]; then
            echo "Something seems wrong with ${path}, please check"
        else
            echo "Skipping ${path}, already exists"
        fi
        return;
    fi

    mkdir -p "${cont_dir}"
    pushd "${cont_dir}"
    echo git clone -b ${llvm_release} $repo "$repo_dir"
    git clone -b ${llvm_release} $repo "$repo_dir"
    popd
}

install_repos() {
    local keys=( ${!submodules[@]} )
    IFS=$'\n' sorted=($(sort <<<"${keys[*]}"))
    unset IFS

    for module in "${sorted[@]}"
    do
        add_submodule "${module}"
    done
}

ask_continue() {
    read -p "continue? " -n 1 -r
    echo    # (optional) move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
        return 0;
    else
        return 1;
    fi
}

check_llvm_root() {
    if [[ ${llvm_root} = $(pwd) ]]; then
        echo "Looks like you are in the llvm root, maybe run in build directory?"
        return 1
    fi
    return 0
}

if [[ $1 = args ]]; then
    echo_cmake_args
elif [[ $1 = repos ]]; then
    install_repos
elif [[ $1 = cmake ]]; then
    if ! check_packages; then
        ask_continue || exit
    fi
    if ! check_llvm_root; then
        ask_continue || exit
    fi

    cmake -G "${build_tool}" $(echo_cmake_args) "${llvm_root}"
elif [[ $1 = pkgs ]]; then
    check_packages
else
    echo "${usage}"
fi
