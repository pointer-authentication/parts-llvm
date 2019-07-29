; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-cpi=full < %s | FileCheck %s

@func = global void ()* @call_func

declare void @call_func()

; CHECK-LABEL: @main
; CHECK: blraa
; CHECK: ret
define i32 @main() {
entry:
  %0 = load void ()*, void ()** @func
  call void %0()
  ret i32 0
}

; CHECK-LABEL: @__pauth_pac_globals
; CHECK-NOT: autia
; CHECK: pacia
; CHECK: .section        .init_array.0,"aw",@init_array
; CHECK: .xword  .L__pauth_pac_globals