; Check metadata for data pointer authentication is added
; RUN: opt -parts-dpi -parts-opt-dpi -parts-opt-dp-args -S < %s | FileCheck %s

; CHECK-LABEL: @simple_pointer
; CHECK: alloca
; CHECK: %call = call i8* @my_simple_alloc(i64 4096)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}
; CHECK: ret i8* %call
define i8* @simple_pointer() {
entry:
  %data = alloca i8*, align 8
  %call = call i8* @my_simple_alloc(i64 4096)
  store i8* %call, i8** %data, align 8
  ret i8* %call
}

declare i8* @my_simple_alloc(i64)
