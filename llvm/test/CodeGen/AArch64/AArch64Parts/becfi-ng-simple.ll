; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-becfi=ng-full < %s | FileCheck %s

; Check that leaf functions aren't instrumented
; CHECK-LABEL: @leaf
; CHECK-NO: pacib
; CHECK-NO: autib
; CHECK: ret
define void @leaf() #0 {
entry:
  ret void
}

; Make sure that LR+FP spill is instrumented
; CHECK-LABEL: @func
; CHECK: adr
; CHECK: pacib
; CHECK: autib
; CHECK: ret
define void @func() #1 {
entry:
    call void @leaf()
    ret void
}

attributes #0 = { noinline nounwind optnone "no-frame-pointer-elim-non-leaf" }
attributes #1 = { noinline nounwind optnone "no-frame-pointer-elim-non-leaf" }
