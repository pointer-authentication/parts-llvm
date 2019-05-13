; ------------------------------------------------------------------------
; Author: Carlos Chinea Perez <carlos.chinea.perez@huawei.com> 
; Copyright (C) 2019 Huawei
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; Test placement of the part.data.pointer.argument intrinsic
;
; RUN: opt -load PartsOpt.so -parts-dpi -parts-opt-dpi -parts-opt-dp-args -S < %s | FileCheck %s
;

define void @test_no_args() {
entry:
; CHECK-NOT: {{.*}}parts.data.pointer.argument{{.*}}
  call void @func_noargs()
  ret void
}

define void @test_data(i64 %d0) {
entry:
; CHECK-NOT: {{.*}}parts.data.pointer.argument{{.*}}
  call void @func_data(i64 %d0)
  ret void
}

define void @test_data_pointer(i64* %d0) {
entry:
; CHECK:  %0 = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK:  call void @func(i64* %0)
  call void @func(i64* %d0)
  ret void
}

define void @test_two_data_pointer(i64* %d0, i32* %d1) {
entry:
; CHECK:  [[DP0:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK:  [[DP1:%[0-9]+]] = call i32* {{.*}}parts.data.pointer.argument{{.*}}(i32* %d1)
; CHECK:  call void @func_two(i64* [[DP0]], i32* [[DP1]])
  call void @func_two(i64* %d0, i32* %d1)
  ret void
}

define void @test_one_data_pointer_one_data(i64* %d0, i32 %d1) {
entry:
; CHECK:  [[DP0:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK-NOT:  {{.*}}parts.data.pointer.argument{{.*}}
; CHECK:  call void @func_one_pointer(i64* [[DP0]], i32 %d1)
  call void @func_one_pointer(i64* %d0, i32 %d1)
  ret void
}

define void @test_n_data_pointers(i64* %d0, i64* %d1, i64* %d2, i64* %d3, i64* %d4, i64* %d5, i64* %d6, i64* %d7) {
; CHECK:  [[DP0:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d0)
; CHECK:  [[DP1:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d1)
; CHECK:  [[DP2:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d2)
; CHECK:  [[DP3:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d3)
; CHECK:  [[DP4:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d4)
; CHECK:  [[DP5:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d5)
; CHECK:  [[DP6:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d6)
; CHECK:  [[DP7:%[0-9]+]] = call i64* {{.*}}parts.data.pointer.argument{{.*}}(i64* %d7)
; CHECK: call void @func_eight(i64* [[DP0]], i64* [[DP1]], i64* [[DP2]], i64* [[DP3]], i64* [[DP4]], i64* [[DP5]], i64* [[DP6]], i64* [[DP7]])
  call void @func_eight(i64* %d0, i64* %d1, i64* %d2, i64* %d3, i64* %d4, i64* %d5, i64* %d6, i64* %d7)
  ret void
}

declare void @func_noargs()
declare void @func_data(i64)
declare void @func(i64*)
declare void @func_two(i64*, i32*)
declare void @func_one_pointer(i64*, i32)
declare void @func_eight(i64 *, i64*, i64 *, i64*, i64 *, i64*, i64 *, i64*)
