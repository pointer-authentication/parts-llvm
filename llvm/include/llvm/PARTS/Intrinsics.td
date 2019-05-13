//===----------------------------------------------------------------------===//
//
// Author: Hans Liljestrand <hans.liljestrand@pm.me>
// Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

def int_pa_pacia : Intrinsic<[llvm_anyptr_ty], [LLVMMatchType<0>, llvm_i64_ty], [IntrNoMem]>;
def int_pa_pacda : Intrinsic<[llvm_anyptr_ty], [LLVMMatchType<0>, llvm_i64_ty], [IntrNoMem]>;
def int_pa_xpacd : Intrinsic<[llvm_anyptr_ty], [LLVMMatchType<0>], [IntrNoMem]>;
def int_pa_autia : Intrinsic<[llvm_anyptr_ty], [LLVMMatchType<0>, llvm_i64_ty], [IntrNoMem]>;
def int_pa_autda : Intrinsic<[llvm_anyptr_ty], [LLVMMatchType<0>, llvm_i64_ty], [IntrNoMem]>;
def int_pa_autcall : Intrinsic<[llvm_anyptr_ty], [LLVMMatchType<0>, llvm_i64_ty], [IntrNoMem], "", [SDNPOutGlue]>;

// Spill handling
def int_parts_data_pointer_argument: Intrinsic<[llvm_anyptr_ty], [LLVMMatchType<0>], [IntrNoMem]>;
