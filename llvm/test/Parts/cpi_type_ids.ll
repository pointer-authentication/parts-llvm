; RUN: opt -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
;
; Further, make sure that we generate separate type_ids for incompatible
; function signatures
;

%struct.video_par = type { i32 }

declare void @OpenAnnexBFile(%struct.video_par*, i8*)
declare void @voider()
declare void @f_int(i8)
declare void @f_intptr(i8*)

@voider_ptr = common hidden local_unnamed_addr global void ()* null, align 8
@func_ptr = common hidden local_unnamed_addr global void (%struct.video_par*, i8*)* null, align 8
@f_int_ptr = global void (i8)* null
@f_intptr_ptr = global void (i8*)* null

define i32 @do_something2() {
  ; Store address of OpenAnnexBFile in func_ptr
  store void (%struct.video_par*, i8*)* @OpenAnnexBFile, void (%struct.video_par*, i8*)** @func_ptr
; CHECK: pacia{{.*}}, i64 [[ID1:-?[0-9]+]]{{\)$}}

  store void ()* @voider, void ()** @voider_ptr
; CHECK-NOT: pacia{{.*}}, i64 [[ID1]]{{\)$}}
; CHECK: pacia{{.*}}, i64 [[ID2:-?[0-9]+]]{{\)$}}

  store void (i8)* @f_int, void (i8)** @f_int_ptr
; CHECK-NOT: pacia{{.*}}, i64 [[ID1]]{{\)$}}
; CHECK-NOT: pacia{{.*}}, i64 [[ID2]]{{\)$}}
; CHECK: pacia{{.*}}, i64 [[ID3:-?[0-9]+]]{{\)$}}

  store void (i8*)* @f_intptr, void (i8*)** @f_intptr_ptr
; CHECK-NOT: pacia{{.*}}, i64 [[ID1]]{{\)$}}
; CHECK-NOT: pacia{{.*}}, i64 [[ID2]]{{\)$}}
; CHECK-NOT: pacia{{.*}}, i64 [[ID3]]{{\)$}}
; CHECK: pacia{{.*}}, i64 [[ID4:-?[0-9]+]]{{\)$}}

  store void (%struct.video_par*, i8*)* @OpenAnnexBFile, void (%struct.video_par*, i8*)** @func_ptr
; CHECK: pacia{{.*}}, i64 [[ID1]]{{\)$}}
  store void ()* @voider, void ()** @voider_ptr
; CHECK: pacia{{.*}}, i64 [[ID2]]{{\)$}}
  store void (i8)* @f_int, void (i8)** @f_int_ptr
; CHECK: pacia{{.*}}, i64 [[ID3]]{{\)$}}
  store void (i8*)* @f_intptr, void (i8*)** @f_intptr_ptr
; CHECK: pacia{{.*}}, i64 [[ID4]]{{\)$}}

  ret i32 0
; CHECK: ret
}



