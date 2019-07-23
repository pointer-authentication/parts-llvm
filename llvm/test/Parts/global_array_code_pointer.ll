; Check that we insert the needed functionality to authenticate a global array of data pointers
; RUN: opt -parts-fecfi -parts-opt-globals -S < %s  | FileCheck %s

@global_array_func = global [4 x void ()*] [ void ()* null, void ()* null, void ()* @func, void ()* null], align 8
; CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @__pauth_pac_globals, i8* null }]

declare void @func()

define i32 @main() {
entry:
; CHECK-NOT:  call void @__pauth_pac_globals()
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load void ()*, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @global_array_func, i64 0, i64 2), align 8
  call void %0()
  ret i32 0
}

; CHECK: define private void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK:  %0 = load void ()*, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @global_array_func, i64 0, i64 2)
; CHECK:  %1 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* %0, i64 2887238391723867588)
; CHECK:  store void ()* %1, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @global_array_func, i64 0, i64 2)
; CHECK:   ret void
; CHECK: }

; CHECK: attributes #0 = { "no-parts"="true" "noinline"="true" }
