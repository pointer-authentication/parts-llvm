; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s
;
; Make sure that a tailcall works, when the FP+LR is not spilled.
;
; void tail_caller(void (*f)(void)) {
;     f();
; }
;

declare void ()* @llvm.pa.autcall.p0f_isVoidf(void ()*, i64) #6

define hidden void @tail_caller(void ()* nocapture %f) local_unnamed_addr #2 {
entry:
  %0 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %f, i64 -8151429658862389052)
  tail call void %0() #7
  ; CHECK: bra
  ; CHECK-NO: ret
  ret void
}


; attributes #2 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "parts-function_id"="9164720793914525444" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }
attributes #6 = { nounwind readnone }
attributes #7 = { nounwind }
