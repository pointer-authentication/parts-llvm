#!/bin/bash
#
# Author: ishkamiel <ishkamiel@hugreal>
# Copyright (C) 2018 ishkamiel <ishkamiel@hugreal>
#
# Distributed under terms of the MIT license.

/usr/bin/find . \
    -path "./lib/Target/*" ! -path "./lib/Target/AArch64/*" -prune -o \
    -path "./cmake" -prune -o \
    -path "./build" -prune -o \
    -path "./cmake-build-debug" -prune -o \
    -path "./build-vscode" -prune -o \
    -path "./unittests" -prune -o \
    -path "./test" -prune -o \
    -path "./projects/libcxx/test/*" -prune -o \
    -path "./projects/compiler-rt/test/*" -prune -o \
    -path "./tools/lld/test/*" -prune -o \
    -path "./tools/clang/test/*" -prune -o \
    -name "*.[chxsS]" -print > cscope.files

    # -path "Testing/*" -prune -o \
