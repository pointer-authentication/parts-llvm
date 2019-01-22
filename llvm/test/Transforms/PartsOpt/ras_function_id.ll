; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: opt -load PartsOpt.so -parts-becfi -parts-opt-ras -S < %s | FileCheck %s
;
; Check that function_id is assigned as a function attribute

define i32 @main(i32 %argc, i8** %argv) {
entry:
  ; CHECK: function_id
  ret i32 0
}

define i32 @do_something() {
   ; CHECK: function_id
   ret i32 0
}
