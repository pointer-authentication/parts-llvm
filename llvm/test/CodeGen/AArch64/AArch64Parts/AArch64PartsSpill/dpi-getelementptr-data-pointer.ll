; Reference test: test/CodeGen/AArch64/ldst-opt.ll
;
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi < %s | FileCheck %s
;

%pre.struct.i32 = type { i32, i32, i32, i32, i32 }

; Function Attrs: nounwind
define i32 @load-pre-indexed-word2(%pre.struct.i32** %this, i1 %cond, %pre.struct.i32* %load2) #0 {
  call void @llvm.parts.data.pointer.argument.p0p0s_pre.struct.i32s(%pre.struct.i32** %this)
  call void @llvm.parts.data.pointer.argument.p0s_pre.struct.i32s(%pre.struct.i32* %load2)
  br i1 %cond, label %if.then, label %if.end

if.then:                                          ; preds = %0
  %load1 = load %pre.struct.i32*, %pre.struct.i32** %this
  %1 = call %pre.struct.i32* @llvm.pa.autda.p0s_pre.struct.i32s(%pre.struct.i32* %load1, i64 -4230606961242437532)
  %gep1 = getelementptr inbounds %pre.struct.i32, %pre.struct.i32* %1, i64 0, i32 1
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %gep1)
  br label %return

if.end:                                           ; preds = %0
  %gep2 = getelementptr inbounds %pre.struct.i32, %pre.struct.i32* %load2, i64 0, i32 2
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %gep2)
; CHECK:  pacda [[PTR:x[0-9]+]], sp
; CHECK:  str [[PTR]], [sp, #8]            // 8-byte Folded Spill
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31},~{sp},~{lr},~{fp}"()
; CHECK:  nop
; CHECK:  ldr [[PTR_RELOAD:x[0-9]+]], [sp, #8]            // 8-byte Folded Reload
; CHECK: autda [[PTR_RELOAD]], sp
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retptr = phi i32* [ %gep1, %if.then ], [ %gep2, %if.end ]
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %retptr)
  %ret = load i32, i32* %retptr
  ret i32 %ret
}

; Function Attrs: nounwind readnone
declare %pre.struct.i32* @llvm.pa.autda.p0s_pre.struct.i32s(%pre.struct.i32*, i64) #1

; Function Attrs: nounwind readnone
declare void @llvm.parts.data.pointer.argument.p0p0s_pre.struct.i32s(%pre.struct.i32**) #1

; Function Attrs: nounwind readnone
declare void @llvm.parts.data.pointer.argument.p0s_pre.struct.i32s(%pre.struct.i32*) #1

; Function Attrs: nounwind readnone
declare void @llvm.parts.data.pointer.argument.p0i32(i32*) #1

attributes #0 = { nounwind }
attributes #1 = { nounwind readnone }
