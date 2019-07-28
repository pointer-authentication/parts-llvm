; Test that code pointer integrity when using parts-cpi=full
;
; RUN: opt -parts-cpi=notype -parts-opt-cpi -S < %s | FileCheck %s
;

%struct.a = type { i32 }
%struct.b = type { i64 }
%struct.c = type { i64 }

declare void @f1a(%struct.a*, i8*)
declare void @f1b(%struct.b*, i8*)
declare void @f1c(%struct.c*, i8*)
declare i8 @f1d(%struct.c*, i8*)

declare void @f2a()
declare void @f2b(i8*)
declare i8 @f2c(i8*)

@p1a = global void (%struct.a*, i8*)* null
@p1b = global void (%struct.b*, i8*)* null
@p1c = global void (%struct.c*, i8*)* null
@p1d = global i8 (%struct.c*, i8*)* null

@p2a = global void ()* null
@p2b = global void (i8*)* null
@p2c = global i8 (i8*)* null

; CHECK-LABEL: @main
define i32 @main() {
; CHECK:   pacia{{.*}}, i64 0)
; CHECK:   store void (%struct.a*, i8*)* {{.*}}, void (%struct.a*, i8*)** @p1a
  store void (%struct.a*, i8*)* @f1a, void (%struct.a*, i8*)** @p1a
; CHECK:   pacia{{.*}}, i64 0)
; CHECK:   store void (%struct.b*, i8*)* {{.*}}, void (%struct.b*, i8*)** @p1b
  store void (%struct.b*, i8*)* @f1b, void (%struct.b*, i8*)** @p1b
; CHECK:   pacia{{.*}}, i64 0)
; CHECK:   store void (%struct.c*, i8*)* {{.*}}, void (%struct.c*, i8*)** @p1c
  store void (%struct.c*, i8*)* @f1c, void (%struct.c*, i8*)** @p1c
; CHECK:   pacia{{.*}}, i64 0)
; CHECK:   store i8 (%struct.c*, i8*)* {{.*}}, i8 (%struct.c*, i8*)** @p1d
  store i8 (%struct.c*, i8*)* @f1d, i8 (%struct.c*, i8*)** @p1d

; CHECK:   pacia{{.*}}, i64 0)
; CHECK:   store void ()* {{.*}}, void ()** @p2a
  store void ()* @f2a, void ()** @p2a
; CHECK:   pacia{{.*}}, i64 0)
; CHECK:   store void (i8*)* {{.*}}, void (i8*)** @p2b
  store void (i8*)* @f2b, void (i8*)** @p2b
; CHECK:   pacia{{.*}}, i64 0)
; CHECK:   store i8 (i8*)* {{.*}}, i8 (i8*)** @p2c
  store i8 (i8*)* @f2c, i8 (i8*)** @p2c

; CHECK: ret i32 0
  ret i32 0
}



