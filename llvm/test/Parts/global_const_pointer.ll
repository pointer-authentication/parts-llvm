; RUN: opt -parts-fecfi -parts-opt-globals -S < %s  | FileCheck %s
;

; CHECK-LABEL: @func1
; CHECK-NOT: constant
@func1 = hidden constant void ()* @printer1, align 8

; CHECK-LABEL: @printer1
declare hidden void @printer1();

; CHECK-LABEL: @__pauth_pac_globals
; CHECK  %0 = load void ()*, void ()** @func1
; CHECK  %1 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* %0, i64 {{[0-9]+}})
; CHECK: store void ()* %1, void ()** @func1
; CHECK: ret void
