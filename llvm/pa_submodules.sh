#!/bin/bash
#
# Author: Hans Liljestrand <liljestrandh@gmail.com>
# Copyright (C) 2018 Aalto University
#
# Distributed under terms of the MIT license.

declare -rg llvm_root=$(dirname $0)
declare -Ag submodules

VERSION=release_60

submodules["tools/clang"]="https://github.com/llvm-mirror/clang.git"
submodules["tools/lld"]="https://github.com/llvm-mirror/lld.git"
submodules["tools/clang/extra"]="https://github.com/llvm-mirror/clang-tools-extra.git"
submodules["tools/lldb"]="https://github.com/llvm-mirror/lldb.git"


add_submodule() {
    local module=$1
    local path=${llvm_root}/$module
    local cont_dir=$(dirname ${path})
    local repo_dir=$(basename ${path})
    local repo=${submodules[$module]}

    if [[ -e ${path} ]]; then
        echo "Skipping ${path}, already exists"
        return;
    fi

    mkdir -p ${cont_dir}
    pushd ${cont_dir}
    git clone -b ${VERSION} $repo $repo_dir
    popd
}

# Sort on key to make sure containing repos are setup first
keys=( ${!submodules[@]} )
IFS=$'\n' sorted=($(sort <<<"${keys[*]}"))
unset IFS

for module in "${sorted[@]}"
do
    add_submodule ${module}
done
