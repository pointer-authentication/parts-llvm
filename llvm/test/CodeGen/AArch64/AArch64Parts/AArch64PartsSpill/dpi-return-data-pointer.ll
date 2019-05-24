; ------------------------------------------------------------------------
; Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com>
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) Huawei Technologies Oy (Finland) Co. Ltd
; Copyright (C) 2019 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi < %s | FileCheck %s
;

define i8* @simple_pointer() {
entry:
  %data = alloca i8*, align 8
  %call = call i8* @my_simple_alloc(i64 4096)
; CHECK:  bl	my_simple_alloc
; CHECK:  pacda	[[PTR_SPILL:x[0-9]+]], sp
; CHECK:  str	[[PTR_SPILL]], [sp]
; CHECK:  nop
  %0 = call i8* @llvm.parts.data.pointer.argument.p0i8(i8* %call)
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31},~{sp},~{lr},~{fp}"() #1
; CHECK:  ldr	[[PTR_RELOAD:x[0-9]+]], [sp]
; CHECK:  autda	[[PTR_RELOAD]], sp
; CHECK:  mov	[[PTR:x[0-9]+]], [[PTR_RELOAD]]
; CHECK:  pacda	[[PTR]], {{x[0-9]+}}
  %1 = call i8* @llvm.pa.pacda.p0i8(i8* %0, i64 -1457565293528958492)
  store i8* %1, i8** %data, align 8
  ret i8* %0
}

declare i8* @my_simple_alloc(i64)

; Function Attrs: nounwind readnone
declare i8* @llvm.pa.pacda.p0i8(i8*, i64) #0

; Function Attrs: nounwind readnone
declare i8* @llvm.parts.data.pointer.argument.p0i8(i8*) #0

attributes #0 = { nounwind readnone }
