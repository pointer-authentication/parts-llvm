; RUN: opt -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
;
; This checks that compatible function signatures produce compatible type_ids, even
; when the compiler internally renames struct names (presumably due to extern definitions).
;

; Two structure of "same" type
%struct.video_par.1072 = type { i32 }
%struct.video_par = type { i32 }

; Function handling our struc type
declare void @OpenAnnexBFile(%struct.video_par*, i8*)

; Global contaiing a function pointer
@func_ptr = common hidden local_unnamed_addr global void (%struct.video_par*, i8*)* null, align 8

define i32 @do_something(void (%struct.video_par.1072*, i8*)* %fptrmod, %struct.video_par* %splain, %struct.video_par.1072* %smod, i32 %b) {

    ; Store address of OpenAnnexBFile in func_ptr
    store void (%struct.video_par*, i8*)* @OpenAnnexBFile, void (%struct.video_par*, i8*)** @func_ptr
    ; CHECK: pacia{{.*}}, i64 [[TYPEID:-?[0-9]+]]{{\)$}}

    ; Load global function pointer can call it
    %1 = load void (%struct.video_par*, i8*)*, void (%struct.video_par*, i8*)** @func_ptr
    call void %1(%struct.video_par* %splain, i8* null)
    ; CHECK: autcall{{.*}}, i64 [[TYPEID]]{{\)$}}

    ; Call the argument function pointer
    call void %fptrmod(%struct.video_par.1072* %smod, i8* null)
    ; CHECK: autcall{{.*}}, i64 [[TYPEID]]{{\)$}}

    ret i32 0
}
