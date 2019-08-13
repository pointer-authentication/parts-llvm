; Test placement of the part.data.pointer.argument intrinsic
;
; RUN: opt -parts-dpi -parts-opt-dpi -parts-opt-dp-args -S < %s | FileCheck %s
;

define void @test_no_args() {
entry:
; CHECK-LABEL: entry:
; CHECK-NOT: {{.*}}parts.data.pointer.argument{{.*}}
  call void @func_noargs()
  ret void
}

define void @test_data(i64 %d0) {
entry:
; CHECK-LABEL: entry:
; CHECK-NOT: {{.*}}parts.data.pointer.argument{{.*}}
  call void @func_data(i64 %d0)
  ret void
}

define void @test_data_pointer(i64* %d0) {
entry:
; CHECK-LABEL: entry:
; CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK:  call void @func(i64* %d0)
  call void @func(i64* %d0)
  ret void
}

define void @test_two_data_pointer(i64* %d0, i32* %d1) {
entry:
; CHECK-LABEL: entry:
; CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}(i32* %d1)
; CHECK:  call void @func_two(i64* %d0, i32* %d1)
  call void @func_two(i64* %d0, i32* %d1)
  ret void
}

define void @test_one_data_pointer_one_data(i64* %d0, i32 %d1) {
entry:
; CHECK-LABEL: entry:
; CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK-NOT:  {{.*}}parts.data.pointer.argument{{.*}}
; CHECK:  call void @func_one_pointer(i64* %d0, i32 %d1)
  call void @func_one_pointer(i64* %d0, i32 %d1)
  ret void
}

define void @test_n_data_pointers(i64* %d0, i64* %d1, i64* %d2, i64* %d3, i64* %d4, i64* %d5, i64* %d6, i64* %d7) {
entry:
; CHECK-LABEL: entry:
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d1)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d2)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d3)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d4)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d5)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d6)
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d7)
; CHECK: call void @func_eight(i64* %d0, i64* %d1, i64* %d2, i64* %d3, i64* %d4, i64* %d5, i64* %d6, i64* %d7)
  call void @func_eight(i64* %d0, i64* %d1, i64* %d2, i64* %d3, i64* %d4, i64* %d5, i64* %d6, i64* %d7)
  ret void
}

define void @test_mix_dp_data(i64* %d0, i64 %d1, i64 %d2, i64 %d3, i64* %d4, i64 %d5, i64 %d6, i64* %d7) {
entry:
; CHECK-LABEL: entry:
; CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK-NEXT: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d4)
; CHECK-NEXT: call void {{.*}}parts.data.pointer.argument{{.*}}(i64* %d7)
; CHECK-NOT:  {{.*}}parts.data.pointer.argument{{.*}}
; CHECK: call void @func_mix(i64* %d0, i64 %d1, i64 %d2, i64 %d3, i64* %d4, i64 %d5, i64 %d6, i64* %d7)
  call void @func_mix(i64* %d0, i64 %d1, i64 %d2, i64 %d3, i64* %d4, i64 %d5, i64 %d6, i64* %d7)
  ret void
}

define void @test_indirect_call(void ()* %f0) {
entry:
; CHECK-LABEL: entry:
; CHECK-NOT:  {{.*}}parts.data.pointer.argument{{.*}}
; CHECK:  call void %f0()
  call void %f0()
  ret void
}

declare void @func_noargs()
declare void @func_data(i64)
declare void @func(i64*)
declare void @func_two(i64*, i32*)
declare void @func_one_pointer(i64*, i32)
declare void @func_eight(i64*, i64*, i64*, i64*, i64*, i64*, i64*, i64*)
declare void @func_mix(i64*, i64, i64, i64, i64*, i64, i64, i64*)
