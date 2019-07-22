; RUN: opt -parts-dpi -parts-opt-dpi -parts-dpi-union-type-punning -S < %s | FileCheck %s
;

%union.fb = type { %struct.foo* }
%struct.foo = type opaque
%struct.bar = type opaque

@foobar = common global %union.fb zeroinitializer, align 8
@ptrbar = common global %struct.bar** null, align 8

define i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call %struct.foo* @alloc_foo()
  store %struct.foo* %call, %struct.foo** getelementptr inbounds (%union.fb, %union.fb* @foobar, i32 0, i32 0), align 8
  %0 = load %struct.bar**, %struct.bar*** @ptrbar, align 8
  %1 = load %struct.bar*, %struct.bar** %0, align 8
  %call1 = call i32 @usebar(%struct.bar* %1)
  %2 = load %struct.bar*, %struct.bar** bitcast (%union.fb* @foobar to %struct.bar**), align 8
  %call2 = call i32 @usebar(%struct.bar* %2)
  %add = add nsw i32 %call1, %call2
  ret i32 %add
; CHECK: call %struct.foo* @llvm.pa.pacda.p0s_struct.foos(%struct.foo* %call, i64 1611178494724599863)
; CHECK: call %struct.bar** @llvm.pa.autda.p0p0s_struct.bars(%struct.bar** {{.*}}, i64 -8775170209848875084)
; CHECK: call %struct.bar* @llvm.pa.autda.p0s_struct.bars(%struct.bar* {{.*}}, i64 1788886855209353903)
; CHECK: call %struct.bar* @llvm.pa.xpacd.p0s_struct.bars(%struct.bar* {{.*}})
}

declare %struct.foo* @alloc_foo()
declare i32 @usebar(%struct.bar*)

; CHECK: declare %struct.foo* @llvm.pa.pacda.p0s_struct.foos(%struct.foo*, i64)
; CHECK: declare %struct.bar** @llvm.pa.autda.p0p0s_struct.bars(%struct.bar**, i64)
; CHECK: declare %struct.bar* @llvm.pa.autda.p0s_struct.bars(%struct.bar*, i64)
; CHECK: declare %struct.bar* @llvm.pa.xpacd.p0s_struct.bars(%struct.bar*)
