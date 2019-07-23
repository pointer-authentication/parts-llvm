; Check metadata for data pointer authentication is added
; RUN: opt -parts-dpi -parts-opt-dpi -S < %s | FileCheck %s

@data = common global i32* null

; CHECK-LABEL: @simple_pointer
; CHEck: %0 = load i32*, i32** %3, align 8
; CHECK: %1 = call {{.*}}autda{{.*}}, i64 [[ID1:-?[0-9]+]])
; CHECK: %2 = call {{.*}}pacda{{.*}}, i64 [[ID1:-?[0-9]+]])
; CHECK: store i32* %2, i32** @data, align 8
; CHECK: ret void
define void @simple_pointer(i32** %val) {
entry:
  %0 = load i32*, i32** %val, align 8
  store i32* %0, i32** @data, align 8
  ret void
}
