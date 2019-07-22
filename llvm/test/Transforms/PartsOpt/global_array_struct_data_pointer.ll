; Check that we insert the needed functionality to authenticate a global array of a struct
; with a data pointer member.
; RUN: opt -parts-dpi -parts-opt-globals -S < %s  | FileCheck %s

%struct.data = type { %struct.data*, i32 }

@global_struct_data = global [1 x %struct.data] [%struct.data { %struct.data* getelementptr inbounds ([1 x %struct.data], [1 x %struct.data]* @global_struct_data, i32 0, i32 0), i32 -559038737 }], align 8
; CHECK: @llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @__pauth_pac_globals, i8* null }]

define i32 @main() {
entry:
; CHECK-NOT: call void @__pauth_pac_globals()
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i32, i32* getelementptr inbounds ([1 x %struct.data], [1 x %struct.data]* @global_struct_data, i64 0, i64 0, i32 1), align 8
  ret i32 %0
}

; CHECK: define private void @__pauth_pac_globals() #0 {
; CHECK: entry:
; CHECK:   %0 = load %struct.data*, %struct.data** getelementptr inbounds ([1 x %struct.data], [1 x %struct.data]* @global_struct_data, i64 0, i64 0, i32 0)
; CHECK:   %1 = call %struct.data* @llvm.pa.pacda.p0s_struct.datas(%struct.data* %0, i64 {{-?[0-9]+}})
; CHECK:   store %struct.data* %1, %struct.data** getelementptr inbounds ([1 x %struct.data], [1 x %struct.data]* @global_struct_data, i64 0, i64 0, i32 0)
; CHECK:   ret void
; CHECK: }

; CHECK: ; Function Attrs: nounwind readnone
; CHECK: declare %struct.data* @llvm.pa.pacda.p0s_struct.datas(%struct.data*, i64) #1

; CHECK: attributes #0 = { "no-parts"="true" "noinline"="true" }
; CHECK: attributes #1 = { nounwind readnone }
