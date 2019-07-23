; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-becfi < %s | FileCheck %s
; XFAIL: *

target triple = "aarch64"

; Check that leaf functions aren't instrumented
define void @leaf() #0 {
entry:
;   CHECK-NO: pacib
    ret void
;   CHECK-NO: autib
}

; Make sure that LR-only spill is instrumented
define void @func() #1 {
entry:
;   CHECK: pacib
    call void @leaf()
;   CHECK: autib
    ret void
}

attributes #0 = { noinline nounwind optnone }
attributes #1 = { noinline nounwind optnone }
