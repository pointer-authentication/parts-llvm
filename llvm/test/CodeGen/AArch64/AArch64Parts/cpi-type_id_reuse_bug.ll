; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s
;

@func = hidden global [4 x void ()*] [void ()* @printer1, void ()* @printer1, void ()* @printer2, void ()* @printer3]

; CHECK-LABEL: @__pauth_pac_globals
; CHECK: ldp [[R1:x[0-9]+]], [[R2:x[0-9]+]], [
; CHECK: ldp [[R3:x[0-9]+]], [[R4:x[0-9]+]], [
; CHECK: movk [[Rmod:x[0-9]+]], #{{[0-9]+}}, lsl
; CHECK: pacia [[R1]], [[Rmod]]
; CHECK: pacia [[R2]], [[Rmod]]
; CHECK: pacia [[R3]], [[Rmod]]
; CHECK-NOT: pacia [[Rmod]], [[Rmod]]
; CHECK: pacia [[R4]], [[Rmod]]
; CHECK: ret
define private void @__pauth_pac_globals() #0 {
entry:
  %0 = load void ()*, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 0)
  %1 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* %0, i64 2887238391723867588)
  store void ()* %1, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 0)
  %2 = load void ()*, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 1)
  %3 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* %2, i64 2887238391723867588)
  store void ()* %3, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 1)
  %4 = load void ()*, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 2)
  %5 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* %4, i64 2887238391723867588)
  store void ()* %5, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 2)
  %6 = load void ()*, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 3)
  %7 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* %6, i64 2887238391723867588)
  store void ()* %7, void ()** getelementptr inbounds ([4 x void ()*], [4 x void ()*]* @func, i64 0, i64 3)
  ret void
}

declare hidden void @printer1() #0;
declare hidden void @printer2() #0;
declare hidden void @printer3() #0;
declare void ()* @llvm.pa.pacia.p0f_isVoidf(void ()*, i64) #11

attributes #0 = { "no-parts"="true" "noinline"="true" }
