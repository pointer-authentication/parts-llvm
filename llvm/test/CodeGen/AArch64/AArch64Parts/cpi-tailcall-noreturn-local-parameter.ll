; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s

define void @tail_call_no_return(i32 (i64, i32)* nocapture %func, i32 %data) local_unnamed_addr #0 {
entry:
  %0 = call i32 (i64, i32)* @llvm.pa.autcall.p0f_i32i64i32f(i32 (i64, i32)* %func, i64 -9071177053261598986)
  %call = tail call i32 %0(i64 -4688846798676181265, i32 %data) #2
  unreachable
}

declare i32 (i64, i32)* @llvm.pa.autcall.p0f_i32i64i32f(i32 (i64, i32)*, i64) #1

attributes #0 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noreturn nounwind }


; CHECK-LABEL: tail_call_no_return
; CHECK: 	stp	x29, x30, [sp, #-16]!
; CHECK: 	mov	x8, x0
; CHECK: 	mov	[[MODSRC:x[1-9]+]], #{{[0-9]+}}
; CHECK: 	movk	[[MODSRC]], #{{[0-9]+}}, lsl #16
; CHECK: 	movk	[[MODSRC]], #{{[0-9]+}}, lsl #32
; CHECK: 	movk	[[MODSRC]], #{{[0-9]+}}, lsl #48
; CHECK-NOT: 	mov	[[MODSRC]], x0
; CHECK-NOT: 	mov	  [[MODDST:x[0-9]+]], [[MODSRC]]
; CHECK: 	mov	x29, sp
; CHECK-NOT: 	blraa	x0, [[MODSRC]]
; CHECK-NOT: 	blraa	[[MODSRC]], [[MODSRC]]
; CHECK: 	blraa	x8, [[MODSRC]]
; CHECK: .Lfunc_end0:
