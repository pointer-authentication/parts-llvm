; Check intrinsic for data pointer authentication is added for select instructions
; RUN: opt -parts-dpi -parts-opt-dpi -parts-opt-dp-args -S < %s | FileCheck %s

define i8* @test_select_ptr(i1 %tst, i8* %lhs, i8* %rhs) {
entry:
;CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}
;CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}
  %res = select i1 %tst, i8* %lhs, i8* %rhs
;CHECK:  call void {{.*}}parts.data.pointer.argument{{.*}}
  call void asm sideeffect "nop", "~{x0},~{x1},~{x2},~{x3},~{x4},~{x5},~{x6},~{x7},~{x8},~{x9},~{x10},~{x11},~{x12},~{x13},~{x14},~{x15},~{x16},~{x17},~{x18},~{x19},~{x20},~{x21},~{x22},~{x23},~{x24},~{x25},~{x26},~{x27},~{x28},~{x29},~{x30},~{x31},~{sp},~{lr},~{fp}"()
  ret i8* %res
}
