; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-cpi=full < %s | FileCheck %s

@func1 = global void ()* @printer1
@func2 = global void ()* @printer2

; CHECK-LABEL: use_func_ptrs
; CHECK: blraa
; CHECK: braa
define void @use_func_ptrs(void ()* nocapture %f1, void ()* nocapture %f2) local_unnamed_addr #0 {
entry:
  tail call void %f1() #1
  tail call void %f2() #1
  ret void
}

; CHECK-LABEL: use_globals
; CHECK: blraa
; CHECK: braa
define void @use_globals() {
entry:
  %0 = load void ()*, void ()** @func1
  tail call void %0()
  %1 = load void ()*, void ()** @func2
  tail call void %1()
  ret void
}

; CHECK-LABEL: main
; CHECK: blr
; CHECK: blr
; CHECK: pacia
; CHECK: str
; CHECK: bl
define hidden i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load void ()*, void ()** @func1
  tail call void %0()

  %1 = load void ()*, void ()** @func2
  tail call void %1()

  store void ()* @printer2, void ()** @func1
  store void ()* @printer1, void ()** @func2

  tail call void @use_func_ptrs(void ()* %0, void ()* %1)
  ret i32 0
}

declare hidden void @printer1()
declare hidden void @printer2()
