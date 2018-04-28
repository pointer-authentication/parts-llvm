# Build instructions

The default setup uses mostly the system LLVM toolchain to build the compiler.
It should mostly include sane defaults, but at this stage I might of course be
missing some stuff. Feel free to modify the setup script as needed.

## Ubuntu 18.04

The `setup_build.sh` script should have defaults workable on a clean Ubuntu
16.04 installation (or with minor modification 18.04, see below).

To check for dependencies run:

```
./setup_build.sh pkgs
```

On a clean Ubuntu 16.04 this should show *cmake clang ccache python-dev
libncurses5-dev swig libedit-dev gcc-5-plugin-dev clang-5.0 libclang-5.0-dev
lld-5.0*

The base LLVM repository needs to be ammended with addtional tools and/or
projects, these are setup in subdirectories (e.g., `tools` and `projects`).
To automatically setup these run:

```
./setup_build.sh repos
```

To modify the installed sub-repos modify `setup_build.sh` and add remove
`submodules` entries. In case there are any failures you might need to manually
fix the directory structure (you can find more details in the [llvm
docs](https://llvm.org/docs/GettingStarted.html#git-mirror)).

To start the actual build process and installation run:

```
cd LLVM_REPOSITORY_ROOT
mkdir build; cd build
../setup_build.sh cmake
make && make install
```

You should end up with a LLVM installation under
`~/opt/llvm/llvm-rel60_debug/`. The `hello_world/Makefile` test code in the
[pa-misc](https://version.aalto.fi/gitlab/platsec/pointer-authentication/pa-misc)
repository is already setup to use this path for immediate testing.

## Ubuntu 18.04

You should be able to repeat the same steps as above on Ubuntu 18.04 by setting
some configuration variables. The LLVM/clang version can be set with
`LLVM_V=x.0`, the GCC version with `GCC_V=x`, and finally the build system with
`BUILD_TOOL=A_cmake_generator`. The following configuration worked on a clean
18.04 installation:

```
cd LLVM_REPO_ROOT
LLVM_V=6.0 GCC_V=7 BUILD_TOOL=Ninja ./setup_build.sh pkgs
# Install missing stuff...
LLVM_V=6.0 GCC_V=7 BUILD_TOOL=Ninja ./setup_build.sh repos
mkdir build; cd build
LLVM_V=6.0 GCC_V=7 BUILD_TOOL=Ninja ../setup_build.sh repos
ninja && ninja install
```


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


