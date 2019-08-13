; Reference: test/CodeGen/AArch64/ldst-opt.ll
;
; Check intrinsic for data pointer authentication is added for phi and getelementptr instructions
; RUN: opt -parts-dpi -parts-opt-dpi -parts-opt-dp-args -S < %s | FileCheck %s

%pre.struct.i32 = type { i32, i32, i32, i32, i32}

define i32 @load-pre-indexed-word2(%pre.struct.i32** %this, i1 %cond,
                                   %pre.struct.i32* %load2) nounwind {
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}
; CHECK: call void {{.*}}parts.data.pointer.argument{{.*}}
  br i1 %cond, label %if.then, label %if.end
if.then:
  %load1 = load %pre.struct.i32*, %pre.struct.i32** %this
  %gep1 = getelementptr inbounds %pre.struct.i32, %pre.struct.i32* %load1, i64 0, i32 1
; CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}
  br label %return
if.end:
  %gep2 = getelementptr inbounds %pre.struct.i32, %pre.struct.i32* %load2, i64 0, i32 2
; CHECK:  call void {{.*}}@llvm.parts.data.pointer.argument{{.*}}
  br label %return
return:
  %retptr = phi i32* [ %gep1, %if.then ], [ %gep2, %if.end ]
; CHECK:  call void {{.*}}@llvm.parts.data.pointer.argument{{.*}}
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31},~{sp},~{lr},~{fp}"()
  %ret = load i32, i32* %retptr
  ret i32 %ret
}
