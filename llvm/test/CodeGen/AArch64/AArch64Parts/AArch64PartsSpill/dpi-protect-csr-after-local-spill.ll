; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi -verify-machineinstrs < %s | FileCheck %s
;

%pre.struct.i32 = type { i32, i32, i32, i32, i32 }

define i32 @load-pre-indexed-word2(%pre.struct.i32** %this, i1 %cond, %pre.struct.i32* %load2) {
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
  br label %return

return:                                           ; preds = %if.end, %if.then
  %retptr = phi i32* [ %gep1, %if.then ], [ %gep2, %if.end ]
  call void @llvm.parts.data.pointer.argument.p0i32(i32* %retptr)
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31},~{sp},~{lr},~{fp}"()
  call void @useptr(i32* %retptr)
  %ret = load i32, i32* %retptr
  ret i32 %ret
}

; CHECK-LABEL:.LBB0_3:
; CHECK:	pacda	x8, sp
; CHECK:	str	x8, [sp, #8]            // 8-byte Folded Spill

; CHECK:	nop

; CHECK:	ldr	x19, [sp, #8]           // 8-byte Folded Reload
; CHECK:	autda	x19, sp
; CHECK:	mov	x0, x19
; CHECK:	pacda	x19, sp
; CHECK:	bl	useptr
; CHECK:	autda	x19, sp
; CHECK:	ldr	w0, [x19]

declare void @useptr(i32*)
declare %pre.struct.i32* @llvm.pa.autda.p0s_pre.struct.i32s(%pre.struct.i32*, i64)
declare void @llvm.parts.data.pointer.argument.p0p0s_pre.struct.i32s(%pre.struct.i32**)
declare void @llvm.parts.data.pointer.argument.p0s_pre.struct.i32s(%pre.struct.i32*)
declare void @llvm.parts.data.pointer.argument.p0i32(i32*)
