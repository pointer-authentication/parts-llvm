; Check intrinsic for data pointer authentication is added for bitcast instructions
;
; Test reference: ../test/CodeGen/AArch64/arm64-ld-from-st.ll
;
; RUN: opt -parts-dpi -parts-opt-dpi -parts-opt-dp-args -S < %s | FileCheck %s

define i64 @Str64Ldr64(i64* nocapture %P, i64 %v, i64 %n) {
entry:
;CHECK:  call void @llvm.parts.data.pointer.argument.p0i64(i64* %P)
  %0 = bitcast i64* %P to i64*
;CHECK:   call void @llvm.parts.data.pointer.argument.p0i64(i64* %0)
  %arrayidx0 = getelementptr inbounds i64, i64* %P, i64 1
;CHECK:   call void @llvm.parts.data.pointer.argument.p0i64(i64* %arrayidx0)
  store i64 %v, i64* %arrayidx0
  %arrayidx1 = getelementptr inbounds i64, i64* %0, i64 1
;CHECK:   call void @llvm.parts.data.pointer.argument.p0i64(i64* %arrayidx1)
  %1 = load i64, i64* %arrayidx1
  ret i64 %1
}
;CHECK: declare void @llvm.parts.data.pointer.argument.p0i64(i64*)
