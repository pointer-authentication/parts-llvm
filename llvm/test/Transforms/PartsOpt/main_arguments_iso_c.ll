; Check intrinsic is added to handle ISO C compliant main() arguments.
; RUN: opt -parts-dpi -parts-opt-mainargs -S < %s | FileCheck %s

; CHECK-LABEL: @main
; CHECK: call void @__pauth_pac_main_args(i32 %argc, i8** %argv)
define i32 @main(i32 %argc, i8** %argv) {
entry:
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  %0 = load i32, i32* %argc.addr, align 4
  %1 = load i8**, i8*** %argv.addr, align 8
  call void @do_something(i32 %0, i8** %1)
  ret i32 0
}

; CHECK-LABEL: @do_something
declare void @do_something(i32, i8**)

; CHECK-LABEL: @__pauth_pac_main_args
; CHECK: call i8* @llvm.pa.pacda.p0i8(i8* %5, i64 {{-?[0-9]+}})
