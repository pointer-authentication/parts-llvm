; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi < %s | FileCheck %s
;

@data = common global i32* null

; CHECK-LABEL: @simple_pointer
; CHECK: mov   [[MOD:x[0-9+]]], #{{[0-9]+}}
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #16
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #32
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #48
; CHECK: autda [[PTR:x[0-9]+]], [[MOD]]
; CHECK: pacda [[PTR]], [[MOD]]
; CHECK: ret
define void @simple_pointer(i32** %val) {
entry:
  %0 = load i32*, i32** %val, align 8
  %1 = call i32* @llvm.pa.autda.p0i32(i32* %0, i64 3615702535277641033)
  %2 = call i32* @llvm.pa.pacda.p0i32(i32* %1, i64 3615702535277641033)
  store i32* %2, i32** @data, align 8
  ret void
}

; Function Attrs: nounwind readnone
declare i32* @llvm.pa.autda.p0i32(i32*, i64) #0

; Function Attrs: nounwind readnone
declare i32* @llvm.pa.pacda.p0i32(i32*, i64) #0

attributes #0 = { nounwind readnone }
