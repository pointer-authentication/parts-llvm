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


declare void @func_noargs()
declare void @func_data(i64)
declare void @func(i64*)
declare void @func_two(i64*, i32*)
