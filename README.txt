# Build instructions

```
cd REPOSORY_PATH
# This will setup nested repositories (modify script if needed)
./init_repos.sh
# Create build directory for cmake
mkdir build
cd build
# Run setup script to setup
./../setup_build.sh setup
```

If something fails, modify setup_build.sh as needed, and check for potentially
needed packages.

## Potentially needed packages (on Ubuntu)

If using the gold plugin (modify according to gcc version):
`gcc-6-plugin-dev`

Seems to be needed even when doxygen is disabled:
`doxygen`

If missing python packages:
`python3-dev python3-pip python3-tk python3-lxml python3-six`


# Original Readme by LLVM
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


