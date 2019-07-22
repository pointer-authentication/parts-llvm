; Check that we insert a constructor function to authenticate global code pointers
; RUN: opt -parts-fecfi -parts-opt-globals -S < %s  | FileCheck %s
@func = global void ()* @call_func, align 8
; CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @__pauth_pac_globals, i8* null }]

declare void @call_func()

define i32 @main() {
entry:
; CHECK-NOT: call void @__pauth_pac_globals()
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load void ()*, void ()** @func, align 8
  call void %0()
  ret i32 0
}

; CHECK: define private void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK:   %0 = load void ()*, void ()** @func
; CHECK:   %1 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* %0, i64 {{-?[0-9]+}})
; CHECK:   store void ()* %1, void ()** @func
; CHECK:   ret void
; CHECK: }

; CHECK: declare void ()* @llvm.pa.pacia.p0f_isVoidf(void ()*, i64) #1

; CHECK: attributes #0 = { "no-parts"="true" "noinline"="true" }
; CHECK: attributes #1 = { nounwind readnone }
