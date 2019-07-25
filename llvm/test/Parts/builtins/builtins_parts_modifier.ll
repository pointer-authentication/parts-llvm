; ------------------------------------------------------------------------
; Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
; Copyright (C) 2019 Huawei Finland Oy
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: opt -parts-opt-builtins -S < %s | FileCheck %s
;
; Check that our public modifier intrinsic is transform into a constant

define i64 @parts_modifier(i64* %value) {
entry:
  %mod = call i64 @llvm.pa.modifier.p0i64(i64* %value)
; CHECK-NOT:  %mod = call i64 @llvm.pa.modifier.p0i64(i64* %value)
; CHECK:  ret i64 {{[-][0-9]+}}
  ret i64 %mod
}

declare i64 @llvm.pa.modifier.p0i64(i64* %value)
