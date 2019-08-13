; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi -verify-machineinstrs < %s | FileCheck %s
;

@glhs = common global i8 0 
@grhs = common global i8 0

declare void @foo(i8*, i8*)
declare void @bar(i8*, i8*)
declare void @llvm.parts.data.pointer.argument.p0i8(i8*)

; CHECK:  tst w0, #0x1
; CHECK:  adrp  x8, grhs
; CHECK:  adrp  x9, glhs
; CHECK:  add x8, x8, :lo12:grhs
; CHECK:  add x9, x9, :lo12:glhs
; CHECK:  csel  x19, x2, x3, ne
; CHECK:  tst w1, #0x1
; CHECK:  csel  x20, x9, x8, ne
; CHECK:  mov x0, x19
; CHECK:  mov x1, x20
; CHECK:  pacda x19, sp
; CHECK:  pacda x20, sp
; CHECK:  bl  foo
; CHECK:  autda x20, sp
; CHECK:  autda x19, sp
; CHECK:  mov x0, x19
; CHECK:  mov x1, x20
; CHECK:  pacda x19, sp
; CHECK:  bl  bar
; CHECK:  autda x19, sp
; CHECK:  mov x0, x19

define i8* @test_select_ptr(i1 %tst, i1 %gtst, i8* %lhs, i8* %rhs) {
entry:
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %lhs)
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %rhs)
  %res = select i1 %tst, i8* %lhs, i8* %rhs
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %res)
  %res2 = select i1 %gtst, i8* @glhs, i8* @grhs
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %res2)
  call void @foo(i8* %res, i8* %res2)
  call void @bar(i8* %res, i8* %res2)
  ret i8* %res
}

; CHECK:  tst w0, #0x1
; CHECK:  adrp  x8, grhs
; CHECK:  adrp  x9, glhs
; CHECK:  add x8, x8, :lo12:grhs
; CHECK:  add x9, x9, :lo12:glhs
; CHECK:  csel  x19, x2, x3, ne
; CHECK:  tst w1, #0x1
; CHECK:  csel  x20, x9, x8, ne
; CHECK:  mov x0, x19
; CHECK:  mov x1, x20
; CHECK:  pacda x19, sp
; CHECK:  pacda x20, sp
; CHECK:  bl  foo
; CHECK:  autda x20, sp
; CHECK:  autda x19, sp
; CHECK:  mov x0, x19
; CHECK:  mov x1, x20
; CHECK:  pacda x19, sp
; CHECK:  pacda x20, sp
; CHECK:  bl  bar
; CHECK:  autda x20, sp
; CHECK:  autda x19, sp
; CHECK:  mov x0, x19
; CHECK:  mov x1, x20
; CHECK:  pacda x19, sp
; CHECK:  bl  bar
; CHECK:  autda x19, sp
; CHECK:  mov x0, x19

define i8* @test_select_ptr2(i1 %tst, i1 %gtst, i8* %lhs, i8* %rhs) {
entry:
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %lhs)
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %rhs)
  %res = select i1 %tst, i8* %lhs, i8* %rhs
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %res)
  %res2 = select i1 %gtst, i8* @glhs, i8* @grhs
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %res2)
  call void @foo(i8* %res, i8* %res2)
  call void @bar(i8* %res, i8* %res2)
  call void @bar(i8* %res, i8* %res2)
  ret i8* %res
}
