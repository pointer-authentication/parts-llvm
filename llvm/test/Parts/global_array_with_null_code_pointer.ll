; Check that we insert the needed functionality to authenticate a global array of data pointers
; RUN: opt -parts-fecfi -parts-opt-globals -S < %s  | FileCheck %s

@global_array_func = global [4 x void ()*] zeroinitializer, align 8
; CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @__pauth_pac_globals, i8* null }]

define i32 @main() {
entry:
; CHECK-NOT:  call void @__pauth_pac_globals()
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load void ()*, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @global_array_func, i64 0, i64 2), align 8
  %1 = icmp ne void()* %0, null
  br i1 %1, label %callf, label %skip

callf:
  call void %0()
  ret i32 0

skip:
  ret i32 0
}

; CHECK: define private void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK-NEXT:   ret void
; CHECK: }

; CHECK: attributes #0 = { "no-parts"="true" "noinline"="true" }
