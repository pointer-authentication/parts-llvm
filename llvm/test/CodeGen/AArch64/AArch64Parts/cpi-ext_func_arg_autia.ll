; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s

@arr = hidden global [5 x i32] [i32 9, i32 8, i32 4, i32 2, i32 1], align 4

declare i32 @cmpfunc(i8* nocapture readonly, i8* nocapture readonly) #0

; CHECK-LABEL: @sorter
; CHECK:    mov    [[MODREG:x[0-9]+]], #{{[0-9]+}}
; CHECK:    movk   [[MODREG]], #{{[0-9]+}}, lsl #16
; CHECK:    movk   [[MODREG]], #{{[0-9]+}}, lsl #32
; CHECK:    movk   [[MODREG]], #{{[0-9]+}}, lsl #48
; CHECK:    autia x3, [[MODREG]]
; CHECK:    b qsort
define void @sorter(i8* %ptr, i64 %len, i64 %el_size, i32 (i8*, i8*)* nocapture %compare) local_unnamed_addr #1 {
entry:
  %0 = call i32 (i8*, i8*)* @llvm.pa.autia.p0f_i32p0i8p0i8f(i32 (i8*, i8*)* %compare, i64 1714582241162187892)
  tail call void @qsort(i8* %ptr, i64 %len, i64 %el_size, i32 (i8*, i8*)* %0) #1
  ret void
}

declare void @qsort(i8*, i64, i64, i32 (i8*, i8*)* nocapture) local_unnamed_addr #2

declare i32 (i8*, i8*)* @llvm.pa.autia.p0f_i32p0i8p0i8f(i32 (i8*, i8*)*, i64) #3

attributes #0 = { norecurse nounwind readonly }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
