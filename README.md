# PARTS 

This is an initial release of the source code for the
[PARTS](https://pointer-authentication.github.io). The project is still in
development and this source code will be updated at a later date, together with
additional testing code.

# Build instructions

The PARTS / LLVM compiler is built using the LLVM cmake build system [LLVM
documentation](https://releases.llvm.org/6.0.0/docs/CMake.html).

To compile PARTS / LLVM for x86 on Ubuntu 16.04 or 18.04 you likely need the
following packages `git cmake python-dev libncurses5-dev swig libedit-dev
libxml2-dev build-essential gcc-7-plugin-dev clang-6 libclang-6-dev lld-6`.

Once installed, you can build the compiler with the following commands

```
git clone --single-branch https://github.com/pointer-authentication/PARTS-llvm.git
cd PARTS-llvm
git clone -b release_60 --single-branch https://github.com/llvm-mirror/clang.git tools/clang --depth 1
mkdir build
cd build
cmake \
          -DCMAKE_INSTALL_PREFIX=${HOME}/opt/PARTS \
          -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=Off \
          -DLLVM_TARGETS_TO_BUILD=AArch64 \
          -DLLVM_BUILD_TOOLS=Off \
          -DLLVM_BUILD_TESTS=Off \ \
          -DLLVM_BUILD_EXAMPLES=Off \
          -DLLVM_BUILD_DOCS=Off \
          -DLLVM_INCLUDE_EXAMPLES=Off \
          -DLLVM_ENABLE_LTO=Off \
          -DLLVM_ENABLE_DOXYGEN=Off \
          -DLLVM_ENABLE_RTTI=Off \
          ..
make -j$(nproc)
```

# Licensing

LLVM and PARTS are open source software. You may freely distribute it under the
terms of the license agreement found in `LICENSE.txt`.

PARTS is however bundled with the SHA-3 implementation from [mbed
TLS](https://tls.mbed.org), distributed under the Apache-2.0 license. The
related code and `LICENSE.txt` file is in the `lib/PARTS-sha3` folder.


# Original LLVM README.txt

```
Low Level Virtual Machine (LLVM)
================================

This directory and its subdirectories contain source code for LLVM,
a toolkit for the construction of highly optimized compilers,
optimizers, and runtime environments.

LLVM is open source software. You may freely distribute it under the terms of
the license agreement found in LICENSE.txt.

Please see the documentation provided in docs/ for further
assistance with LLVM, and in particular docs/GettingStarted.rst for getting
started with LLVM and docs/README.txt for an overview of LLVM's
documentation setup.

If you are writing a package for LLVM, see docs/Packaging.rst for our
suggestions.
```


