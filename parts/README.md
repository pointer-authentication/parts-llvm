# Build instructions

The default setup uses the system LLVM toolchain to build the compiler. The
build configuration should be adequate for now and is relatively fast, in
particular, the LLVM build uses shared libraries which allow for quick partial
recompiles during development.

Setup is done by `setup_build.sh`, which is tested on clean Ubuntu 16.04 and
18.04 installations. To see usage just run it without arguments. Options are
pretty limited, but it does allow some shell variables to modify the behavior
(e.g., `build_tool="Unix Makefiles"` switches from `ninja` to `make`. 

Bigger changes require manual editing of the script. For instance, to add more
LLVM sub-repos edit `setup_build.sh` and add/remove entries as needed. These
"sub-repos" are essentially various tools and projects for LLVM (e.g., clang
itself is a tool).

**Note that the setup script is locatoin dependent, always keep the script in
the LLVM root directory.**

## Ubuntu 16.04 (for 18.04, see next section)

Clone and checkout [**pa-simplelr**
branch](https://version.aalto.fi/gitlab/platsec/pointer-authentication/pa-llvm/tree/pa-simplelr)
from our PA LLVM repository (or a mainline LLVM **release_60** and manually
copy in `setup_build.sh`).

Check and install dependencies:

```
./setup_build.sh pkgs | xargs sudo apt-get -y install
```

The base LLVM repository needs to be amended with additional tools and/or
projects, these are setup in subdirectories (e.g., `./tools/*` and
`./projects/*`). To automatically setup these run:

```
./setup_build.sh repos
```

In case there are any failures you might need to manually fix the directory
structure (you can find more details in the [LLVM
docs](https://llvm.org/docs/GettingStarted.html#git-mirror)).

To start the actual build process and installation run:

```
mkdir build; cd build
../setup_build.sh cmake
ninja && ninja install
```

You should end up with a LLVM installation under
`~/opt/llvm/llvm-rel60_debug/`. The `hello_world/Makefile` test code in the
[pa-misc](https://version.aalto.fi/gitlab/platsec/pointer-authentication/pa-misc)
repository is already setup to use this path for immediate testing.

## Ubuntu 18.04

Setup the following environmental variables to use newer and available compiler
versions:

```
export system_llvm=6.0
export system_gcc=7
```

Then just follow the 16.04 instructions above.

## cmake options (e.g., for CLion use)

You can get the cmake variables that are set by running:

```
# Set env variables as needed (e.g., system_llvm or system_gcc).
./setup_build.sh args
```

You can use this output to setup the CLion IDE, for instance. Or perhaps just
to troubleshoot.


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


