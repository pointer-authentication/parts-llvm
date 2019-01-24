; ------------------------------------------------------------------------
; Author: Hans Liljestrand <hans.liljestrand@pm.me>
; Copyright (C) 2018 Secure Systems Group, Aalto University <ssg.aalto.fi>
;
; This file is distributed under the University of Illinois Open Source
; License. See LICENSE.TXT for details.
; ------------------------------------------------------------------------
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s
;
; Test a case where the instrumentation produces a mov x0, xzr before the call,
; thus causing it to bra to NULL.
;

%struct.video_par = type { i64, i64 }

declare hidden void @func() #3

define hidden void @test_funcptr(void (%struct.video_par*, i8*)* nocapture %func_ptr) local_unnamed_addr #3 {
  ;
  ; This works okay, i.e., x0 is saved before it is zeroed:
  ; tail call void %func_ptr(%struct.video_par* null, i8* null) #9
  ;

  ; CHECK: mov [[REGISTER:x[0-9]+]], x0
  %1 = call void (%struct.video_par*, i8*)* @llvm.pa.autia.p0f_isVoidp0s_struct.video_parsp0i8f(void (%struct.video_par*, i8*)* %func_ptr, i64 8293111894729183960)
  tail call void %1(%struct.video_par* null, i8* null) #9
  ; CHECK-NO: mov [[REGISTER]], xzr
  ; CHECK: braa [[REGISTER]]
  ret void
}

define hidden void @test_funcptr2(void (%struct.video_par*, i8*)* nocapture %func_ptr) local_unnamed_addr #3 {
  ; CHECK: mov [[REGISTER:x[0-9]+]], x0
  call void () @func() #3
  ; CHECK: bl
  %1 = call void (%struct.video_par*, i8*)* @llvm.pa.autia.p0f_isVoidp0s_struct.video_parsp0i8f(void (%struct.video_par*, i8*)* %func_ptr, i64 8293111894729183960)
  tail call void %1(%struct.video_par* null, i8* null) #9
  ; CHECK-NO: mov [[REGISTER]], xzr
  ; CHECK: braa [[REGISTER]]
  ret void
}

declare void (%struct.video_par*, i8*)* @llvm.pa.autia.p0f_isVoidp0s_struct.video_parsp0i8f(void (%struct.video_par*, i8*)*, i64) #3

attributes #3 = { nounwind readnone "no-frame-pointer-elim-non-leaf" }
attributes #9 = { nounwind }
