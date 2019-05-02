; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi < %s | FileCheck %s
;

; CHECK-LABEL: @spill_data_pointer
; CHECK: mov   [[MOD:x[0-9+]]], #{{[0-9]+}}
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #16
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #32
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #48
; CHECK: autda [[PTR:x[0-9]+]], [[MOD]]
; CHECK: pacda [[PTR]], [[MOD]]
; CHECK: ret
define void @spill_data_pointer(i64* %d0, i64* %d1, i64* %d2, i64 %d3, i64 %d4, i64 %d5, i64 %d6, i64 %d7, i64* %dataptronstack) {
entry:
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31}"() nounwind
  call void (i64*, i64*, i64*, i64, i64, i64, i64, i64, i64*) @func(i64* %d0, i64* %d1, i64* %d2, i64 %d3, i64 %d4, i64 %d5, i64 %d6, i64 %d7, i64* %dataptronstack)
  ret void
}

; Function Attrs: nounwind readnone
declare void @func(i64*, i64*, i64*, i64, i64, i64, i64, i64, i64*)
