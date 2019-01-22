; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-becfi < %s | FileCheck %s
;
; Make sure that a tailcall works, when the FP+LR is not spilled.
;
; void tail_caller(void (*f)(void)) {
;     f();
; }
;

target triple = "aarch64"

define hidden void @tail_called() #0 {
entry:
  ret void
}

; Function Attrs: noinline nounwind
define hidden void @tail_caller(void ()* nocapture %f) local_unnamed_addr #1 {
entry:
  ; CHECK-NO: pacib
  tail call void %f()
  ; CHECK-NO: autib
  ; CHECK: br
  ret void
}

attributes #0 = { noinline nounwind optnone "no-frame-pointer-elim-non-leaf" "parts-function_id"="1" }
attributes #0 = { noinline nounwind optnone "no-frame-pointer-elim-non-leaf" "parts-function_id"="2" }
