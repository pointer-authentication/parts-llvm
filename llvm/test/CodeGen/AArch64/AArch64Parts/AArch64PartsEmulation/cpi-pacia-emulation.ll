; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi --parts-dummy < %s | FileCheck %s
;
; Simple pacia test for code pointer integrity

target triple = "aarch64"

@func1 = hidden global void ()* @printer1, align 8
@.str = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@__FUNCTION__.printer1 = private unnamed_addr constant [9 x i8] c"printer1\00", align 1
@func2 = hidden global void ()* @printer2, align 8
@__FUNCTION__.printer2 = private unnamed_addr constant [9 x i8] c"printer2\00", align 1

declare i32 @printf(i8*, ...) #1

define hidden void @printer1() #0 {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @__FUNCTION__.printer1, i32 0, i32 0))
  ret void
}

define hidden void @printer2() #0 {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @__FUNCTION__.printer2, i32 0, i32 0))
  ret void
}

define hidden i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %0 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* @printer2, i64 -8151429658862389052)
  store void ()* %0, void ()** @func1, align 8

  ret i32 0
}

; Function Attrs: nounwind readnone
declare void ()* @llvm.pa.pacia.p0f_isVoidf(void ()*, i64) #4

attributes #0 = { noinline nounwind optnone }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone }

; CHECK:  adrp   [[SRC:x[0-9]+]], printer2
; CHECK:  mov   [[MODSRC:x[0-9]+]], #{{[0-9]+}}
; CHECK:  movk  [[MODSRC]], #{{[0-9]+}}, lsl #16
; CHECK:  movk  [[MODSRC]], #{{[0-9]+}}, lsl #32
; CHECK:  movk  [[MODSRC]], #{{[0-9]+}}, lsl #48
; CHECK-NOT: mov   [[SRC]], [[SRC]]
; CHECK:  eor   [[SRC]], [[SRC]], #0x3ffff0003ffff
; CHECK:  eor   [[SRC]], [[SRC]], #0x3f003f003f003f
; CHECK:  eor   [[SRC]], [[SRC]], #0x8001800180018001
; CHECK:  eor   [[SRC]], [[SRC]], [[MODSRC]]
; CHECK:  str   [[SRC]], [x{{[0-9]+}}]
