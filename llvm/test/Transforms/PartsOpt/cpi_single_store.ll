; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: opt -load PartsOpt.so -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
;
; Test a single store.
;

declare void @voider()

@fptr = global void ()* null, align 8

define i32 @func() {
  store void ()* @voider, void ()** @fptr
; CHECK: pacia{{.*}}, i64 [[ID1:-?[0-9]+]]{{\)$}}
; CHECK: store

  ret i32 0
; CHECK: ret
}



