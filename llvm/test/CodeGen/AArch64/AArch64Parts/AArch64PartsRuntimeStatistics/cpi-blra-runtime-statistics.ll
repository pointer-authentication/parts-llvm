; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi --parts-stats < %s | FileCheck %s

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
; CHECK:  bl  __parts_count_code_ptr_branch
; CHECK:  blra{{[ab]}} x0
