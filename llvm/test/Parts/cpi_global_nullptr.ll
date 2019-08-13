; RUN: opt -parts-fecfi -parts-opt-cpi -parts-opt-globals -S < %s | FileCheck %s
;
; Make sure that NULL code pointers aren't instrumented
;

; CHECK: @g_fptr = hidden local_unnamed_addr global void ()* null
@g_fptr = hidden local_unnamed_addr global void ()* null

;
; This mostly just illustrates where the problem manifests, equivalent C is about:
;
;   if (ptr != NULL) ptr();
;
; CHECK-LABEL: @caller
; CHECK: %cmp = icmp eq void ()* %ptr, null
; CHECK: br i1 %cmp, label %if.end, label %if.then
; CHECK: autcall{{.*}} i64 {{-?[0-9]+}})
; CHECK: tail call
; CHECK: br label %if.end
; CHECK: ret void
define hidden void @caller(void ()* %ptr) local_unnamed_addr {
entry:
  %cmp = icmp eq void ()* %ptr, null
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  tail call void %ptr()
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  ret void
}

;
; This function should work, it makes sure that assigning NULL to a function pointer, or passing
; NULL as a function pointer argument, doesn't result in PACing.
;
; CHECK-LABEL: @okayish
; CHECK-NOT: pacia
; CHECK: store void ()* null, void ()** @g_fptr
; CHECK-NOT: pacia
; CHECK: tail call void @caller(void ()* null)
; CHECK-NOT: pacia
; CHECK: ret void
define hidden void @okayish() local_unnamed_addr {
  %1 = load void ()*, void ()** @g_fptr, align 8
  tail call void @caller(void ()* %1)
  store void ()* null, void ()** @g_fptr, align 8
  tail call void @caller(void ()* null)
  ret void
}

;
; And this is the reason we put this test in, i.e., global function pointers with NULL value get PACed!
;
; CHECK-LABEL: @__pauth_pac_globals()
; CHECK-NEXT: entry
; CHECK-NEXT: ret void

