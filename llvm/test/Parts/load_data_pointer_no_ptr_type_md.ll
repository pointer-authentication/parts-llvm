; Check pointer type metadata is not generated if PARTS DPI is not enabled
; RUN: opt -parts-opt-dpi -S < %s | FileCheck %s

define i8 @simple_pointer(i8* %val) {
entry:
  %0 = load i8, i8* %val
; CHECK-NOT: %0 = load i8, i8* %val, !PartsTypeMetadata !0
  ret i8 %0
}
; CHECK-NOT: !0 = !{!"PartsTypeMetadata", i64 0, i1 true, i1 false, i1 false, i1 false, i1 false}
