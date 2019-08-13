; RUN: opt -parts-dpi -parts-opt-dpi -S < %s | FileCheck %s
;

%struct.longptr = type { %union.anon }
%union.anon = type { i64* }

declare void @LoadAssign([20 x i64]* %arraybase)

; CHECK-LABEL: @fail
define hidden void @fail(i64* %ptr) {
entry:
; CHECK: %abase = alloca %struct.longptr
  %abase = alloca %struct.longptr

; CHECK: %ptrs = getelementptr inbounds %struct.longptr, %struct.longptr* %abase, i32 0, i32 0
; CHECK: %p = bitcast %union.anon* %ptrs to i64**
; CHECK: pacda{{.*}}i64 [[ID:-?[0-9]+]])
; CHECK: store i64* %0, i64** %p, align 8
  %ptrs = getelementptr inbounds %struct.longptr, %struct.longptr* %abase, i32 0, i32 0
  %p = bitcast %union.anon* %ptrs to i64**
  store i64* %ptr, i64** %p, align 8

; CHECK: %ptrs1 = getelementptr inbounds %struct.longptr, %struct.longptr* %abase, i32 0, i32 0
; CHECK: %ap = bitcast %union.anon* %ptrs1 to [20 x [20 x i64]]**
; CHECK: %1 = load [20 x [20 x i64]]*, [20 x [20 x i64]]** %ap, align 8
; CHECK-NOT: autda{{.*}}i64 [[ID]])
; CHECK: autda{{.*}}i64 {{-?[0-9]+}})
; CHECK: %arraydecay = getelementptr inbounds [20 x [20 x i64]], [20 x [20 x i64]]* %2, i32 0, i32 0
; CHECK: call void @LoadAssign([20 x i64]* %arraydecay)
  %ptrs1 = getelementptr inbounds %struct.longptr, %struct.longptr* %abase, i32 0, i32 0
  %ap = bitcast %union.anon* %ptrs1 to [20 x [20 x i64]]**
  %0 = load [20 x [20 x i64]]*, [20 x [20 x i64]]** %ap, align 8
  %arraydecay = getelementptr inbounds [20 x [20 x i64]], [20 x [20 x i64]]* %0, i32 0, i32 0
  call void @LoadAssign([20 x i64]* %arraydecay)

; CHECK: ret void
  ret void
}
