; RUN: opt -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
;
; Catch a but in code where a bitcast is used to convert direct function
; argument into a void pointer. E.g:
;
;   void func(void *bar);
;   void hack(void);
;   fun(hack);
;


declare void @fun(i8* %bar) #0;
declare void @hack() #0;

define void @func() #1 {
entry:
  call void @fun(i8* bitcast (void ()* @hack to i8*))
; CHECK: pacia{{.*}}, i64 [[ID1:-?[0-9]+]]{{\)$}}
; CHECK: call
  ret void
; CHECK: ret
}

attributes #0 = { noinline nounwind optnone "parts-cpi" }
attributes #1 = { noinline nounwind optnone }
