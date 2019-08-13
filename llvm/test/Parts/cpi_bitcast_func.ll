; RUN: opt -parts-fecfi -parts-opt-cpi -S < %s | FileCheck %s
;
; It seems that in some cases the compiler produces weird calls, wherein the
; called function is actually bitcast to match the input parameters, not the
; other way around (as I would have suspected). A bug causes this to be
; interpreted as an indirect call.

%struct.video_par.1072 = type { i32 }
%struct.video_par = type { i32 }

declare void @initBitsFile(%struct.video_par* %a, i32 %b)

define i32 @do_something(%struct.video_par.1072* %a, i32 %b) {
    call void bitcast (void (%struct.video_par*, i32)* @initBitsFile to void (%struct.video_par.1072*, i32)*)(%struct.video_par.1072* %a, i32 %b)
    ; CHECK:            call
    ; CHECK-NOT:        autia
    ; CHECK:            ret
    ret i32 0
}
