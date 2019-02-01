; Check intrinsic is added to handle ISO C compliant main() arguments.
; RUN: opt -load PartsOpt.so -parts-dpi -parts-opt-mainargs -S < %s | FileCheck %s

define i32 @main(i32 %argc, i8** %argv) {
entry:
; CHECK: call void @__pauth_pac_main_args(i32 %argc, i8** %argv, i64 {{-?[0-9]+}})
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  %0 = load i32, i32* %argc.addr, align 4
  %1 = load i8**, i8*** %argv.addr, align 8
  call void @do_something(i32 %0, i8** %1)
  ret i32 0
}

declare void @do_something(i32, i8**)
; CHECK: declare void @__pauth_pac_main_args(i32, i8**, i64)
