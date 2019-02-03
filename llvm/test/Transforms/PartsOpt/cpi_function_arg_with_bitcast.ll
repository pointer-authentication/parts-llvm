; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: opt -load PartsOpt.so -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
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

define void @func() #0 {
entry:
  call void @fun(i8* bitcast (void ()* @hack to i8*))
; CHECK: pacia{{.*}}, i64 [[ID1:-?[0-9]+]]{{\)$}}
; CHECK: call
  ret void
; CHECK: ret
}

; attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #0 = { noinline nounwind optnone }
