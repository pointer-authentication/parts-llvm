; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi < %s | FileCheck %s
;

@data = common global i32* null

; CHECK-LABEL: @simple_pointer
; CHECK: mov   [[MOD:x[0-9+]]], #{{[0-9]+}}
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #16
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #32
; CHECK: movk  [[MOD]], #{{[0-9]+}}, lsl #48
; CHECK: autda [[PTR:x[0-9]+]], [[MOD]]
; CHECK: pacda [[PTR]], sp
; CHECK: str [[PTR]], [sp, #8]            // 8-byte Folded Spill
; CHECK: nop

; CHECK: ldr [[RELOAD:x[0-9]+]], [sp, #8]            // 8-byte Folded Reload

; CHECK: autda [[RELOAD]], sp

; CHECK: pacda [[RELOAD]], {{x[0-9]+}}

; CHECK: ret

define void @simple_pointer(i32** %val) {
entry:
  %0 = load i32*, i32** %val, align 8
  %1 = call i32* @llvm.pa.autda.p0i32(i32* %0, i64 3615702535277641033)
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31},~{sp},~{lr},~{fp}"() #1
  %2 = call i32* @llvm.pa.pacda.p0i32(i32* %1, i64 3615702535277641033)
  store i32* %2, i32** @data, align 8
  ret void
}

; Function Attrs: nounwind readnone
declare i32* @llvm.pa.autda.p0i32(i32*, i64) #0

; Function Attrs: nounwind readnone
declare i32* @llvm.pa.pacda.p0i32(i32*, i64) #0

attributes #0 = { nounwind readnone }
attributes #1 = { nounwind }
