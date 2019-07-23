; Check metadata for data pointer authentication is added
; RUN: opt -parts-dpi -parts-opt-dpi -S < %s | FileCheck %s

; CHECK-LABEL: @simple_pointer
; CHEck: %0 = load i8*, i8** %val
; CHECK: %1 = call i8* {{.*}}autda{{.*}}, i64 [[ID1:-?[0-9]+]])
; CHECK: ret i8* %1
define i8* @simple_pointer(i8** %val) {
entry:
  %0 = load i8*, i8** %val
  ret i8* %0
}
