; ------------------------------------------------------------------------
; Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2019 Huawei Technologies Oy (Finland) Co. Ltd
; Copyright (C) 2019 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi < %s | FileCheck %s
;

;CHECK:	tst	w0, #0x1
;CHECK:	csel	[[PTR:x[0-9]+]], x1, x2, ne
;CHECK:	pacda	[[PTR]], sp
;CHECK:	str	[[PTR]], [sp, #8]            // 8-byte Folded Spill
;CHECK:	nop
;CHECK:	ldr	[[PTR_RELOAD:x[0-9]+]], [sp, #8]            // 8-byte Folded Reload
;CHECK:	autda	[[PTR_RELOAD]], sp

define i8* @test_select_ptr(i1 %tst, i8* %lhs, i8* %rhs) {
entry:
  %0 = call i8* @llvm.parts.data.pointer.argument.p0i8(i8* %lhs)
  %1 = call i8* @llvm.parts.data.pointer.argument.p0i8(i8* %rhs)
  %res = select i1 %tst, i8* %0, i8* %1
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31},~{sp},~{lr},~{fp}"()
  ret i8* %res
}

; Function Attrs: nounwind readnone
declare i8* @llvm.parts.data.pointer.argument.p0i8(i8*) #0

attributes #0 = { nounwind readnone }
