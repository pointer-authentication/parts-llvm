; Check that code pointers in initalized structs are a'okay
; e.g., this should result in a signed NULL pointer a->func
;
; struct foo {
;   struct foo *next;
;   void (*func)(void);
; };
;
; struct foo a = { 0 };
;
; RUN: opt -O2 -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
; XFAIL: *

%struct.foo = type { %struct.foo*, void ()* }

; CHECK-LABEL: @main
; CHECK: entry:
; CHECK:   %a = alloca %struct.foo
; CHECK:   %0 = bitcast %struct.foo* %a to i8*
; CHECK:   call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %0)
; CHECK:   call void @llvm.memset.p0i8.i64(i8* nonnull align 8 %0, i8 0, i64 16, i1 false)
; CHECK:   {{.*}}pacia{{.*}}
; CHECK:   call void @test_code_NULL(%struct.foo* nonnull %a)
; CHECK:   call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %0)
; CHECK:   ret i32 0
define i32 @main() {
entry:
  %a = alloca %struct.foo
  %0 = bitcast %struct.foo* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %0)
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 %0, i8 0, i64 16, i1 false)
  call void @test_code_NULL(%struct.foo* nonnull %a)
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %0)
  ret i32 0
}

declare void @test_code_NULL(%struct.foo* %a)
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)
declare void ()* @llvm.pa.autcall.p0f_isVoidf(void ()*, i64)
