; Check code pointer NULL check
; e.g., this hsould work with or without signing:
;
; struct foo {
;   struct foo *next;
;   void (*func)(void);
; };
;
; if (a->next != NULL)
;   use(a->next);
;
; RUN: opt -O2 -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
; XFAIL: *

%struct.foo = type { %struct.foo*, void ()* }

; CHECK-LABEL: @test_data_NULL
; CHECK: entry:
; CHECK:   %next = getelementptr inbounds %struct.foo, %struct.foo* %a, i64 0, i32 0
; CHECK:   %0 = load %struct.foo*, %struct.foo** %next, align 8, !tbaa !8
; CHECK:   %1 = call %struct.foo* @llvm.pa.autda.p0s_struct.foos(%struct.foo* %0, i64 {{[0-9]+}})
; CHECK:   %cmp = icmp eq %struct.foo* %1, null
; CHECK:   br i1 %cmp, label %if.end, label %if.then
; CHECK: if.then:
; CHECK:   tail call void @test_data_NULL(%struct.foo* nonnull %1)
; CHECK:   br label %if.end
; CHECK: if.end:
; CHECK:   tail call void @test_code_NULL(%struct.foo* nonnull %a)
; CHECK:   ret void
define void @test_data_NULL(%struct.foo* nocapture readonly %a) {
entry:
  %next = getelementptr inbounds %struct.foo, %struct.foo* %a, i64 0, i32 0
  %0 = load %struct.foo*, %struct.foo** %next, align 8
  %cmp = icmp eq %struct.foo* %0, null
  br i1 %cmp, label %if.end, label %if.then
if.then:
  tail call void @test_data_NULL(%struct.foo* nonnull %0)
  br label %if.end
if.end:
   tail call void @test_code_NULL(%struct.foo* nonnull %a)
   ret void
 }

declare void @test_code_NULL(%struct.foo* nocapture readonly %a)
declare %struct.foo* @llvm.pa.autda.p0s_struct.foos(%struct.foo*, i64)
