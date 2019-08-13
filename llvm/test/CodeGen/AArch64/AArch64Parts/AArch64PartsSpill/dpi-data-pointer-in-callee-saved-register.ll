; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi -verify-machineinstrs < %s | FileCheck %s
;

;CHECK:	tst	w0, #0x1
;CHECK:	csel	x19, x1, x2, ne
;CHECK:	mov	x0, x19
;CHECK: pacda x19, sp
;CHECK:	bl	foo
;CHECK: autda x19, sp
;CHECK:	mov	x0, x19
;CHECK:	ldp	x19, x30, [sp], #16
;CHECK:	ret

define i8* @test_select_ptr(i1 %tst, i8* %lhs, i8* %rhs) {
entry:
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %lhs)
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %rhs)
  %res = select i1 %tst, i8* %lhs, i8* %rhs
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %res)
  call void @foo(i8* %res)
  ret i8* %res
}

declare void @foo(i8*)

;CHECK:	tst	w0, #0x1
;CHECK:	csel	x19, x1, x2, ne
;CHECK:	mov	x0, x19
;CHECK: pacda x19, sp
;CHECK:	bl	foo
;CHECK: autda x19, sp
;CHECK:	mov	x0, x19
;CHECK: pacda x19, sp
;CHECK:	bl  bar
;CHECK: autda x19, sp
;CHECK:	mov	x0, x19
;CHECK:	ldp	x19, x30, [sp], #16
;CHECK:	ret

define i8* @test_select_ptr2(i1 %tst, i8* %lhs, i8* %rhs) {
entry:
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %lhs)
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %rhs)
  %res = select i1 %tst, i8* %lhs, i8* %rhs
  call void @llvm.parts.data.pointer.argument.p0i8(i8* %res)
  call void @foo(i8* %res)
  call void @bar(i8* %res)
  ret i8* %res
}

declare void @bar(i8*)

; Function Attrs: nounwind
declare void @llvm.parts.data.pointer.argument.p0i8(i8*) #0

attributes #0 = { nounwind }
