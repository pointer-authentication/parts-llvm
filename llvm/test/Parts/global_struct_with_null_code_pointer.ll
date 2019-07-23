; Check that we insert a constructor function to authenticate global struct with code pointers members
; RUN: opt -parts-fecfi -parts-opt-globals -S < %s  | FileCheck %s

%struct.callback = type { void (i32)*, i32 }

@global_struct_code = global %struct.callback { void (i32)* null, i32 -559038737 }, align 8
; CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @__pauth_pac_globals, i8* null }]

; declare void @call_func(i32)

; Function Attrs: noinline nounwind optnone
define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load void (i32)*, void (i32)** getelementptr inbounds (%struct.callback, %struct.callback* @global_struct_code, i32 0, i32 0), align 8
  %1 = load i32, i32* getelementptr inbounds (%struct.callback, %struct.callback* @global_struct_code, i32 0, i32 1), align 8
  call void %0(i32 %1)
  ret i32 0
}

; CHECK: define private void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK-NOT:   %0 = load void (i32)*, void (i32)** getelementptr inbounds (%struct.callback, %struct.callback* @global_struct_code, i32 0, i32 0)
; CHECK-NOT:   %1 = call void (i32)* @llvm.pa.pacia.p0f_isVoidi32f(void (i32)* %0, i64 {{-?[0-9]+}})
; CHECK-NOT:   store void (i32)* %1, void (i32)** getelementptr inbounds (%struct.callback, %struct.callback* @global_struct_code, i32 0, i32 0)
; CHECK:   ret void
; CHECK: }

; CHECK: attributes #0 = { "no-parts"="true" "noinline"="true" }
