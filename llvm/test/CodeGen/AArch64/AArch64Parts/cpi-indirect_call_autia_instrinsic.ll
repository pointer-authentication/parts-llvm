; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s

@indfunc = global void ()* @extern_func, align 8

declare void @extern_func() #0

define void @indcall() #1 {
entry:
  %0 = load void ()*, void ()** @indfunc, align 8
  %1 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %0, i64 2887238391723867588)
  call void %1()
  ret void
}

declare void ()* @llvm.pa.autcall.p0f_isVoidf(void ()*, i64) #2

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

; Generated code to check

; CHECK-LABEL: @indcall
; CHECK:	adrp	[[PTR:x[0-9]+]], indfunc
; CHECK:	add	[[PTR]], [[PTR]], :lo12:indfunc
; CHECK:	ldr	[[PTR]], {{\[}}[[PTR]]{{\]}}
; CHECK:    mov    [[MODREG:x[0-9]+]], #{{[0-9]+}}
; CHECK:    movk   [[MODREG]], #{{[0-9]+}}, lsl #16
; CHECK:    movk   [[MODREG]], #{{[0-9]+}}, lsl #32
; CHECK:    movk   [[MODREG]], #{{[0-9]+}}, lsl #48
; CHECK: 	mov	    x23, [[MODREG]]
; CHECK:	blraa	[[PTR]], x23
; CHECK:	ret
