; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s
; ModuleID = '<stdin>'
source_filename = "tailcall.c"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64"

; Function Attrs: noreturn nounwind
define void @tail_call_no_return(i32 (i32)* nocapture %func, i32 %data) local_unnamed_addr #0 {
entry:
  %0 = call i32 (i32)* @llvm.pa.autia.p0f_i32i32f(i32 (i32)* %func, i64 2855011782346598141)
  %call = tail call i32 %0(i32 %data) #2
  unreachable
}

; Function Attrs: nounwind readnone
declare i32 (i32)* @llvm.pa.autia.p0f_i32i32f(i32 (i32)*, i64) #1

attributes #0 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noreturn nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 2f27999df400d17b33cdd412fdd606a88208dfcc) (https://github.com/llvm-mirror/llvm.git 32e838d4b32640b300ab16adf4026a379a821ad5)"}

; CHECK: 	.text
; CHECK: 	.file	"tailcall.c"
; CHECK: 	.globl	tail_call_no_return     // -- Begin function tail_call_no_return
; CHECK: 	.p2align	2
; CHECK: 	.type	tail_call_no_return,@function
; CHECK: tail_call_no_return:                    // @tail_call_no_return
; CHECK: // %bb.0:                               // %entry
; CHECK: 	stp	x29, x30, [sp, #-16]!   // 8-byte Folded Spill
; CHECK: 	mov	x8, #59133
; CHECK: 	movk	x8, #60498, lsl #16
; CHECK: 	movk	x8, #2582, lsl #32
; CHECK: 	movk	x8, #10143, lsl #48
; CHECK-NOT: 	mov	x8, x0
; CHECK: 	mov	w0, w1
; CHECK: 	mov	x29, sp
; CHECK: 	blraa	x0, x8
; CHECK: .Lfunc_end0:
; CHECK: 	.size	tail_call_no_return, .Lfunc_end0-tail_call_no_return
