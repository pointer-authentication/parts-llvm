; Check that we insert a function to authenticate global data pointers at the beginning of main
; RUN: opt -load LLVMPtrTypeMDPass.so -parts-dpi -pauth-markglobals -S < %s  | FileCheck %s
@global_data = internal global i8* null, align 8

define i32 @main() {
entry:
; CHECK:  call void @__pauth_pac_globals()
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i8*, i8** @global_data, align 8
  call void @use_data(i8* %0)
  ret i32 0
}

declare void @use_data(i8*)

; CHECK: define void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK:   %0 = load i8*, i8** @global_data
; CHECK:   %1 = call i8* @llvm.pa.pacda.p0i8(i8* %0, i64 3284146682194183030)
; CHECK:   store i8* %1, i8** @global_data
; CHECK:   ret void
; CHECK: }

; CHECK: ; Function Attrs: nounwind readnone
; CHECK: declare i8* @llvm.pa.pacda.p0i8(i8*, i64) #1

; CHECK: attributes #0 = { "no-parts"="true" }
; CHECK: attributes #1 = { nounwind readnone }
