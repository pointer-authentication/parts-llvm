; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpp=notype < %s | FileCheck %s

@data = common global i32* null

; CHECK-LABEL: @deref_data
; CHECK-DAG: ldr [[PTR:x[0-9]+]], [x0]
; CHECK-DAG: mov [[MOD:x[0-9+]]], xzr
; CHECK: autda [[PTR:x[0-9]+]], [[MOD]]
; CHECK: ldr
; CHECK: ret
define i32 @deref_data(i32** %val) {
entry:
  %0 = load i32*, i32** %val
  %1 = load i32, i32* %0
  ret i32 %1
}
