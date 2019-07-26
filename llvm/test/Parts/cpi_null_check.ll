; Check code pointer NULL check
; e.g., this hsould work with or without signing:
;
; struct foo {
;   struct foo *next;
;   void (*func)(void);
; };
;
; if (a->func != NULL)
;   a->func();
;
; RUN: opt -O2 -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
; XFAIL: *

%struct.foo = type { %struct.foo*, void ()* }

; CHECK-LABEL: @test_code_NULL
; CHECK: entry:
; CHECK:   %func = getelementptr inbounds %struct.foo, %struct.foo* %a, i64 0, i32 1
; CHECK:   %0 = load void ()*, void ()** %func
; CHECK:   [[stripped:%[0-9]+]] = {{.*}}autia{{.*}}
; CHECK:   %cmp = icmp eq void ()* [[stripped]], null
; CHECK:   br i1 %cmp, label %if.end, label %if.then
; CHECK: if.then
; CHECK:   %[[fptr:%[0-9]+]] = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %0, i64 {{[0-9]+}})
; CHECK:   tail call void [[fptr]]()
; CHECK:   br label %if.end
; CHECK: if.end
; CHECK:   ret void
define hidden void @test_code_NULL(%struct.foo* nocapture readonly %a) local_unnamed_addr {
entry:
  %func = getelementptr inbounds %struct.foo, %struct.foo* %a, i64 0, i32 1
  %0 = load void ()*, void ()** %func, align 8
  %cmp = icmp eq void ()* %0, null
  br i1 %cmp, label %if.end, label %if.then
if.then:
  tail call void %0() #4
  br label %if.end
if.end:
  ret void
}
