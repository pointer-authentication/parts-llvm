; Test lowering of PARTS xpacd instrinsic
; RUN: llc -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-dpi < %s | FileCheck %s

%struct.bar = type opaque
%union.fb = type { %struct.foo* }
%struct.foo = type opaque

define %struct.bar* @getbar(%union.fb* nocapture readonly %foobar) local_unnamed_addr {
entry:
  %bar = bitcast %union.fb* %foobar to %struct.bar**
  %0 = load %struct.bar*, %struct.bar** %bar, align 8
  %1 = call %struct.bar* @llvm.pa.xpacd.p0s_struct.bars(%struct.bar* %0)
; CHECK: xpacd  {{x[0-9]+}}
  ret %struct.bar* %1
}

declare %struct.bar* @llvm.pa.xpacd.p0s_struct.bars(%struct.bar*)
