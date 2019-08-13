; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-becfi==full < %s | FileCheck %s
; XFAIL: *
;
; Make sure that a tailcall works, when the FP+LR has been spilled.
;
; void tail_caller(void (*f)(void)) {
;     printf("calling\n");
;     f();
; }
;

target triple = "aarch64"

declare i32 @puts(i8* nocapture readonly) local_unnamed_addr #1

@str.2 = private unnamed_addr constant [8 x i8] c"calling\00"

define hidden void @tail_called() #0 {
entry:
  ret void
}

define hidden void @tail_caller(void ()* nocapture %f) local_unnamed_addr #0 {
entry:
  ; CHECK: pacib
  %puts = tail call i32 @puts(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @str.2, i64 0, i64 0))
  tail call void %f()
  ; CHECK: autib
  ; CHECK: br
  ret void
}

attributes #0 = { noinline nounwind optnone }
attributes #1 = { nounwind }
