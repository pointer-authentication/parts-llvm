; Check that we insert a function to authenticate global data pointers at the beginning of main
; RUN: opt -parts-dpi -parts-opt-globals -S < %s  | FileCheck %s
@global_data = internal global i8* null, align 8
; CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @__pauth_pac_globals, i8* null }]

define i32 @main() {
entry:
; CHECK-NOT: call void @__pauth_pac_globals()
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i8*, i8** @global_data, align 8
  call void @use_data(i8* %0)
  ret i32 0
}

declare void @use_data(i8*)

; CHECK: define private void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK:   %0 = load i8*, i8** @global_data
; CHECK:   %1 = call i8* @llvm.pa.pacda.p0i8(i8* %0, i64 {{-?[0-9]+}})
; CHECK:   store i8* %1, i8** @global_data
; CHECK:   ret void
; CHECK: }

; CHECK: ; Function Attrs: nounwind readnone
; CHECK: declare i8* @llvm.pa.pacda.p0i8(i8*, i64) #1

; CHECK: attributes #0 = { "no-parts"="true" "noinline"="true" }
; CHECK: attributes #1 = { nounwind readnone }
