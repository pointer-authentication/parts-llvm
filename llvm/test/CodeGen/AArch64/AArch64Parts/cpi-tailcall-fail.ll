; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s
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

  %1 = call void (%struct.video_par*, i8*)* @llvm.pa.autcall.p0f_isVoidp0s_struct.video_parsp0i8f(void (%struct.video_par*, i8*)* %func_ptr, i64 8293111894729183960)
  tail call void %1(%struct.video_par* null, i8* null) #9
  ret void
}
; CHECK-LABEL: @test_funcptr
; CHECK: mov    [[MODREG:x[0-9]+]], #{{[0-9]+}}
; CHECK: movk   [[MODREG]], #{{[0-9]+}}, lsl #16
; CHECK: movk   [[MODREG]], #{{[0-9]+}}, lsl #32
; CHECK: movk   [[MODREG]], #{{[0-9]+}}, lsl #48
; CHECK: mov [[PTR:x[0-9]+]], x0
; CHECK-NOT: mov	  [[MODDST:x[0-9]+]], [[MODREG]]
; CHECK-NOT: mov [[PTR]], xzr
; CHECK: braa [[PTR]], [[MODREG]]

define hidden void @test_funcptr2(void (%struct.video_par*, i8*)* nocapture %func_ptr) local_unnamed_addr #3 {
  ; CHECK-LABEL: @test_funcptr2
  call void () @func() #3
  ; CHECK: str    x0, [sp, #8]
  ; CHECK: bl
  %1 = call void (%struct.video_par*, i8*)* @llvm.pa.autcall.p0f_isVoidp0s_struct.video_parsp0i8f(void (%struct.video_par*, i8*)* %func_ptr, i64 8293111894729183960)
  tail call void %1(%struct.video_par* null, i8* null) #9
  ; CHECK: mov    [[MODREG:x[0-9]+]], #{{[0-9]+}}
  ; CHECK: ldr    x2, [sp, #8]
  ; CHECK: movk   [[MODREG]], #{{[0-9]+}}, lsl #16
  ; CHECK: movk   [[MODREG]], #{{[0-9]+}}, lsl #32
  ; CHECK: movk   [[MODREG]], #{{[0-9]+}}, lsl #48
  ; CHECK-NOT: mov    [[MODDST:x[0-9]+]], [[MODREG]]
  ; CHECK-NOT: mov [[REGISTER]], xzr
  ; CHECK: braa   x2, [[MODREG]]
  ret void
}

declare void (%struct.video_par*, i8*)* @llvm.pa.autcall.p0f_isVoidp0s_struct.video_parsp0i8f(void (%struct.video_par*, i8*)*, i64) #3

attributes #3 = { nounwind readnone "no-frame-pointer-elim-non-leaf" }
attributes #9 = { nounwind }
