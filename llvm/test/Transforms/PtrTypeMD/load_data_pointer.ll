; Check metadata for data pointer authentication is added
; RUN: opt -load LLVMPtrTypeMDPass.so -parts-dpi -ptr-type-md-pass -S < %s | FileCheck %s

define i8 @simple_pointer(i8* %val) {
entry:
  %0 = load i8, i8* %val
; CHECK: %0 = load i8, i8* %val, !PartsTypeMetadata !0
  ret i8 %0
}
; CHECK: !0 = !{!"PartsTypeMetadata", i64 0, i1 true, i1 false, i1 false, i1 false, i1 false}
