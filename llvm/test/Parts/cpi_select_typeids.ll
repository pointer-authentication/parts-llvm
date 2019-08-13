; RUN: opt -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
;
; Make sure that select is handled correctly.
;

%struct.video_par = type { i32 }

declare void @voider1()
declare void @voider2()

@voider_ptr = global void ()* null

define i32 @do_something1(i1 %arg) {
  ; Store address of OpenAnnexBFile in func_ptr
  %1 = select i1 true, void ()* @voider1, void ()* @voider2
  store void ()* %1, void ()** @voider_ptr
; CHECK: pacia{{.*}}, i64 [[ID1:-?[0-9]+]]{{\)$}}
; CHECK: pacia{{.*}}, i64 [[ID1]]
; CHECK: select
; CHECK: store

  ret i32 0
; CHECK: ret
}



