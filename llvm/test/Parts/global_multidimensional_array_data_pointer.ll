; Check that we insert the needed functionality to authenticate a global multidimensional array of data pointers
; RUN: opt -parts-dpi -parts-opt-globals -S < %s  | FileCheck %s

@global_data = global i8 127, align 1
@global_array_data = global [3 x [4 x i8*]] [[4 x i8*] [i8* @global_data, i8* @global_data, i8* null, i8* @global_data], [4 x i8*] [i8* @global_data, i8* @global_data, i8* null, i8* @global_data], [4 x i8*] [i8* @global_data, i8* @global_data, i8* null, i8* @global_data]], align 8
; CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @__pauth_pac_globals, i8* null }]

define i32 @main() {
entry:
; CHECK-NOT: call void @__pauth_pac_globals()
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 1), align 8
  %1 = load i8, i8* %0, align 1
  %conv = zext i8 %1 to i32
  ret i32 %conv
}

; CHECK: define private void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK:   %0 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 0)
; CHECK:   %1 = call i8* @llvm.pa.pacda.p0i8(i8* %0, i64 {{-?[0-9]+.$}}
; CHECK:   store i8* %1, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 0)
; CHECK:   %2 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 1)
; CHECK:   %3 = call i8* @llvm.pa.pacda.p0i8(i8* %2, i64 {{-?[0-9]+}})
; CHECK:   store i8* %3, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 1)
; CHECK:   %4 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 2)
; CHECK:   %5 = call i8* @llvm.pa.pacda.p0i8(i8* %4, i64 {{-?[0-9]+}})
; CHECK:   store i8* %5, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 2)
; CHECK:   %6 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 3)
; CHECK:   %7 = call i8* @llvm.pa.pacda.p0i8(i8* %6, i64 {{-?[0-9]+}})
; CHECK:   store i8* %7, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 0, i64 3)
; CHECK:   %8 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 0)
; CHECK:   %9 = call i8* @llvm.pa.pacda.p0i8(i8* %8, i64 {{-?[0-9]+}})
; CHECK:   store i8* %9, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 0)
; CHECK:   %10 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 1)
; CHECK:   %11 = call i8* @llvm.pa.pacda.p0i8(i8* %10, i64 {{-?[0-9]+}})
; CHECK:   store i8* %11, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 1)
; CHECK:   %12 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 2)
; CHECK:   %13 = call i8* @llvm.pa.pacda.p0i8(i8* %12, i64 {{-?[0-9]+}})
; CHECK:   store i8* %13, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 2)
; CHECK:   %14 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 3)
; CHECK:   %15 = call i8* @llvm.pa.pacda.p0i8(i8* %14, i64 {{-?[0-9]+}})
; CHECK:   store i8* %15, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 1, i64 3)
; CHECK:   %16 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 0)
; CHECK:   %17 = call i8* @llvm.pa.pacda.p0i8(i8* %16, i64 {{-?[0-9]+}})
; CHECK:   store i8* %17, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 0)
; CHECK:   %18 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 1)
; CHECK:   %19 = call i8* @llvm.pa.pacda.p0i8(i8* %18, i64 {{-?[0-9]+}})
; CHECK:   store i8* %19, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 1)
; CHECK:   %20 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 2)
; CHECK:   %21 = call i8* @llvm.pa.pacda.p0i8(i8* %20, i64 {{-?[0-9]+}})
; CHECK:   store i8* %21, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 2)
; CHECK:   %22 = load i8*, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 3)
; CHECK:   %23 = call i8* @llvm.pa.pacda.p0i8(i8* %22, i64 {{-?[0-9]+}})
; CHECK:   store i8* %23, i8** getelementptr inbounds ([3 x [4 x i8*]], [3 x [4 x i8*]]* @global_array_data, i64 0, i64 2, i64 3)
; CHECK:   ret void
; CHECK: }

; CHECK: ; Function Attrs: nounwind readnone
; CHECK: declare i8* @llvm.pa.pacda.p0i8(i8*, i64) #1

; CHECK: attributes #0 = { "no-parts"="true" "noinline"="true" }
; CHECK: attributes #1 = { nounwind readnone }
