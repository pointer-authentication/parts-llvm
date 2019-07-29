; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpp=full < %s | FileCheck %s

@global_data = global i8* null

; CHECK-LABEL: @main
; CHECK: bl __pauth_pac_main_args
; CHECK: ret
define i32 @main(i32 %argc, i8** %argv) {
entry:
  call void @use_data(i8** %argv)
  ret i32 0
}

declare void @use_data(i8**)

; CHECK-LABEL: @__pauth_pac_main_args
; CHECK-NOT: autda
; CHECK: pacda
; CHECK-NOT: pacda

