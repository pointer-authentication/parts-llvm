; RUN: opt -parts-opt-builtins -S < %s | FileCheck %s
;
; Check that our public modifier intrinsic is transform into a constant

define i64 @parts_modifier(i64* %value) {
entry:
  %mod = call i64 @llvm.pa.modifier.p0i64(i64* %value)
; CHECK-NOT:  %mod = call i64 @llvm.pa.modifier.p0i64(i64* %value)
; CHECK:  ret i64 [[MODI64:[-]{0,1}[0-9]+]]
  ret i64 %mod
}

%struct.foo = type { i64, i64* }

define i64 @parts_struct_modifier(%struct.foo* %value) {
entry:
  %mod = call i64 @llvm.pa.modifier.p0struct.foo(%struct.foo* %value)
; CHECK-NOT:  %mod = call i64 @llvm.pa.modifier.p0i64(%struct.foo* %value)
; CHECK-NOT:  ret i64 [[MODI64]]
; CHECK:  ret i64 {{[-]{0,1}[0-9]+}}
  ret i64 %mod
}

declare i64 @llvm.pa.modifier.p0i64(i64* %value)
declare i64 @llvm.pa.modifier.p0struct.foo(%struct.foo* %value)
