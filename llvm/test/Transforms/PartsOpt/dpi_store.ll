; Check metadata for data pointer authentication is added
; RUN: opt -parts-dpi -parts-opt-dpi -S < %s | FileCheck %s

; CHECK-LABEL: @simple_pointer
; CHECK: alloca
; CHECK: %call = call i8* @my_simple_alloc(i64 4096)
; CHECK: %0 = call i8* {{.*}}pacda{{.*}}, i64 [[ID1:-?[0-9]+]])
; CHECK: store
define i8* @simple_pointer() {
entry:
  %data = alloca i8*, align 8
  %call = call i8* @my_simple_alloc(i64 4096)
  store i8* %call, i8** %data, align 8
  %0 = load i8*, i8** %data, align 8
  ret i8* %0
}

declare i8* @my_simple_alloc(i64)
