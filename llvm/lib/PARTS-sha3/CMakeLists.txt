#===----------------------------------------------------------------------===//
#
# Author: Hans Liljestrand <hans@liljestrand.dev>
# Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===//

add_llvm_library(PARTSsha3
  keccak.c
  sha3.c

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/PARTS-sha3/include
  )

set_property(TARGET Parts PROPERTY LLVM_SYSTEM_LIBS)
