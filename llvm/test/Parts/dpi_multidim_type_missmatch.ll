; RUN: opt -parts-dpi -parts-opt-dpi -S < %s | FileCheck %s
;
; Based on:
;
;   int main() {
;      long *arr = (long *)malloc((sizeof(long) * ASSIGNROWS * ASSIGNCOLS));
;      free(arr);
;   }

; CHECK-LABEL: @main
; CHECK: %arr = alloca i64*, align 8
; CHECK: %call = call noalias i8* @malloc(i64 3200)
; CHECK: pacda
; CHECK: store
; CHECK: call void @free(i8* %call)
; CHECK: ret void
define hidden void @main() {
entry:
  %arr = alloca i64*, align 8
  %call = call noalias i8* @malloc(i64 3200)
  %0 = bitcast i8* %call to i64*
  store i64* %0, i64** %arr, align 8
  call void @free(i8* %call) #2
  ret void
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64)

; Function Attrs: nounwind
declare void @free(i8*)
