; RUN: opt -parts-becfi=full -parts-opt-ras -S < %s | FileCheck %s
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
