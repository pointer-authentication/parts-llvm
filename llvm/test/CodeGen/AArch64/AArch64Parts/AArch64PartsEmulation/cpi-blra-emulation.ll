; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi --parts-dummy < %s | FileCheck %s

declare void ()* @llvm.pa.autcall.p0f_isVoidf(void ()*, i64) #6

define hidden void @tail_caller(void ()* nocapture %f) local_unnamed_addr #2 {
entry:
  %0 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %f, i64 -8151429658862389052)
  call void %0()
  ret void
}
; CHECK:  mov   [[MODSRC:x[0-9]+]], #{{[0-9]+}}
; CHECK:  movk  [[MODSRC]], #{{[0-9]+}}, lsl #16
; CHECK:  movk  [[MODSRC]], #{{[0-9]+}}, lsl #32
; CHECK:  movk  [[MODSRC]], #{{[0-9]+}}, lsl #48
; CHECK-NOT: blra{{[ab]}} x0
; CHECK:  eor   x0, x0, #0x3ffff0003ffff
; CHECK:  eor   x0, x0, #0x3f003f003f003f
; CHECK:  eor   x0, x0, #0x8001800180018001
; CHECK:  eor   x0, x0, [[MODSRC]]
; CHECK:  blr   x0
