; RUN: opt -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
;
; Test a single store.
;

declare void @voider()

@fptr = global void ()* null, align 8

define i32 @func() {
  store void ()* @voider, void ()** @fptr
; CHECK: pacia{{.*}}, i64 [[ID1:-?[0-9]+]]{{\)$}}
; CHECK: store

  ret i32 0
; CHECK: ret
}



