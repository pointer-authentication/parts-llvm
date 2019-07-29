; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpp=full < %s | FileCheck %s

@global_data = global i8* null

; CHECK-LABEL: @main
; CHECK: autda
; CHECK: ret
define i32 @main() {
entry:
  %0 = load i8*, i8** @global_data
  call void @use_data(i8* %0)
  ret i32 0
}

declare void @use_data(i8*)

; CHECK-LABEL: @__pauth_pac_globals
; CHECK-NOT: autda
; CHECK: pacda
; CHECK: .section        .init_array.0,"aw",@init_array
; CHECK: .xword  .L__pauth_pac_globals

