# Build instructions

## Default configuration

The default setup_script.sh uses clang, the gold linker and creates shared
libraries. It does NOT include documentation, tests, and examples. Tools (e.g.,
clang) are not included in the default make target, but are compilable and
installable with (e.g., `make install-clang`).

## Prerequisites on Ubuntu 16.04

A freshly installed and upgraded amd64 Ubuntu 16.04 requires the following packages:

```
sudo apt install git cmake clang ccache python-dev libncurses5-dev swig libedit-dev
```

## Modifying

The defaults should work on Ubuntu 16.04 but you can:

1) modify `init_repos.sh` to change included tools/projects.
2) modify `setup_script.sh` to modify cmake/build configuration.

## To install

```
cd REPOSORY_PATH
# Brigng in sub tools/projects (including clang)
./init_repos.sh
# Create and enter build directory
mkdir build && cd build
# Run setup script to do setup
./../setup_build.sh setup
# Compile the base
make
# Compile clang
make clang
# Install
make install
make install-clang
```

## Applying changes


# Original LLVM README.md below

At least in our simple test so far backend changes can be compiled and applied
with:

```
make install-LLVMAArch64CodeGen
```

Optimization passes, or other changes, might require more re-compilation.
Ideally the build system should detect needed recompilation and keep it to
a minimum even if just doing a plain `make` but I have not confirmed this at
this point.


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


