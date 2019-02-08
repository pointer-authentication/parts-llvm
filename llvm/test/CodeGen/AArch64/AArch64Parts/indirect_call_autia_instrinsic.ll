; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s
; ModuleID = '<stdin>'
source_filename = "indirect_call.c"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64"

@indfunc = global void ()* @extern_func, align 8

declare void @extern_func() #0

; Function Attrs: noinline nounwind optnone
define void @indcall() #1 {
entry:
  %0 = load void ()*, void ()** @indfunc, align 8
  %1 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %0, i64 2887238391723867588)
  call void %1()
  ret void
}

; Function Attrs: nounwind readnone
declare void ()* @llvm.pa.autcall.p0f_isVoidf(void ()*, i64) #2

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 2f27999df400d17b33cdd412fdd606a88208dfcc) (https://github.com/llvm-mirror/llvm.git 32e838d4b32640b300ab16adf4026a379a821ad5)"}

; Generated code to check

; CHECK: indcall:                                // @indcall
; CHECK: // %bb.0:                               // %entry
; CHECK: 	stp	x29, x30, [sp, #-16]!   // 8-byte Folded Spill
; CHECK: 	mov	x29, sp
; CHECK: 	adrp	x8, indfunc
; CHECK: 	add	x8, x8, :lo12:indfunc
; CHECK: 	ldr	x8, [x8]
; CHECK: 	mov	x9, #15812
; CHECK: 	movk	x9, #8338, lsl #16
; CHECK: 	movk	x9, #34821, lsl #32
; CHECK: 	movk	x9, #10257, lsl #48
; CHECK: 	str	x19, [x29, #16]         // 8-byte Folded Spill
; CHECK: 	mov	x19, x9
; CHECK: 	mov	x8, x8
; CHECK: 	blraa	x8, x19
; CHECK-NEXT: 	ldr	x19, [x29, #16]         // 8-byte Folded Reload
; CHECK: 	ldp	x29, x30, [sp], #16     // 8-byte Folded Reload
; CHECK: 	ret
