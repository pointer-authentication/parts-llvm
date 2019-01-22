; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-becfi < %s | FileCheck %s
; XFAIL: *
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

define hidden void @tail_caller(void ()* nocapture %f) local_unnamed_addr #0 {
entry:
  ; CHECK-NO: pacib
  %0 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* @tail_called, i64 -8151429658862389052)
  ; CHECK-NO: autia
  tail call void %0() #1
  ; CHECK: braa
  ; CHECK-NO: br
  ret void
}

declare void ()* @llvm.pa.pacia.p0f_isVoidf(void ()*, i64) #5

attributes #0 = { noinline nounwind optnone "no-frame-pointer-elim-non-leaf" }
attributes #5 = { nounwind readnone }
