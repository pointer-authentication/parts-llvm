; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s

define void @tail_call_no_return(i32 (i32)* nocapture %func, i32 %data) local_unnamed_addr #0 {
entry:
  %0 = call i32 (i32)* @llvm.pa.autcall.p0f_i32i32f(i32 (i32)* %func, i64 2855011782346598141)
  %call = tail call i32 %0(i32 %data) #2
  unreachable
}

declare i32 (i32)* @llvm.pa.autcall.p0f_i32i32f(i32 (i32)*, i64) #1

attributes #0 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noreturn nounwind }

; CHECK-LABEL: @tail_call_no_return
; CHECK: 	mov	  [[MODSRC:x[0-9]+]], #59133
; CHECK: 	movk	[[MODSRC]], #60498, lsl #16
; CHECK: 	movk	[[MODSRC]], #2582, lsl #32
; CHECK: 	mov	x8, x0
; CHECK: 	movk	[[MODSRC]], #10143, lsl #48
; CHECK-NOT: 	mov	  [[MODDST:x[0-9]+]], [[MODSRC]]
; CHECK: 	mov	w0, w1
; CHECK: 	mov	x29, sp
; CHECK: 	blraa	x8, [[MODSRC]]
; CHECK: .Lfunc_end0:
