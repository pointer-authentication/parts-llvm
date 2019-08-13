; RUN: opt -parts-dpi -parts-opt-dpi -S < %s | FileCheck %s
;
; based on misc/code/cpi_type_ids
;
; This currently only contains two minimal checks, needs to be updated once
; implementation goes along.
;

; ModuleID = 'dpi_type_ids.out.linked.bc'
source_filename = "llvm-link"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64"

%struct.a_struct = type { i32, i32 }
%struct.b_struct = type { i64, i64, i64 }

@ga = hidden global %struct.a_struct zeroinitializer, align 4
@gb = hidden global %struct.b_struct zeroinitializer, align 8
@gpa = hidden local_unnamed_addr global %struct.a_struct* @ga, align 8
@gpb = hidden local_unnamed_addr global %struct.b_struct* @gb, align 8
@.str = private unnamed_addr constant [8 x i8] c"%p%p%p\0A\00", align 1

; Function Attrs: noinline norecurse nounwind readnone
define hidden void @voider() local_unnamed_addr #0 {
entry:
  ret void
}

; Function Attrs: nounwind
define hidden i32 @main(i32 %argc, i8** nocapture readnone %argv) local_unnamed_addr #1 {
entry:
  %la = alloca i64, align 8
  %lb = alloca %struct.b_struct, align 8
  %a = alloca i64, align 8
  %b = alloca i32, align 4
  %0 = bitcast i64* %la to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #4
  store i64 0, i64* %la, align 8
  %1 = bitcast %struct.b_struct* %lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* nonnull %1) #4
  call void @llvm.memset.p0i8.i64(i8* nonnull %1, i8 0, i64 24, i32 8, i1 false)
  %2 = bitcast i64* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %2) #4
  %3 = load %struct.a_struct*, %struct.a_struct** @gpa, align 8, !tbaa !3
; CHECK: pacda{{.*}}, i64 [[APTR_ID:-?[0-9]+]]{{\)$}}
  %a1 = getelementptr inbounds %struct.a_struct, %struct.a_struct* %3, i64 0, i32 0
  %4 = load i32, i32* %a1, align 4, !tbaa !7
  %conv = sext i32 %4 to i64
  %5 = load %struct.b_struct*, %struct.b_struct** @gpb, align 8, !tbaa !3
; CHECK-NOT: pacda{{.*}}, i64 [[APTR_ID]]{{\)$}}
; CHECK: pacda{{.*}}, i64 [[BPTR_ID:-?[0-9]+]]{{\)$}}
  %a2 = getelementptr inbounds %struct.b_struct, %struct.b_struct* %5, i64 0, i32 0
  %6 = load i64, i64* %a2, align 8, !tbaa !10
  %add = add nsw i64 %6, %conv
  store i64 %add, i64* %a, align 8, !tbaa !13
  %7 = bitcast i32* %b to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %7) #4
  %conv3 = trunc i64 %add to i32
  store i32 %conv3, i32* %b, align 4, !tbaa !14
  store i64* %la, i64** bitcast (%struct.a_struct** @gpa to i64**), align 8, !tbaa !3
  store %struct.b_struct* %lb, %struct.b_struct** @gpb, align 8, !tbaa !3
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i64 0, i64 0), i64* nonnull %a, i32* nonnull %b, %struct.a_struct* nonnull @ga)
  %8 = load i64, i64* %a, align 8, !tbaa !13
  %conv4 = trunc i64 %8 to i32
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %7) #4
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %2) #4
  call void @llvm.lifetime.end.p0i8(i64 24, i8* nonnull %1) #4
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #4
  ret i32 %conv4
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #2

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

attributes #0 = { noinline norecurse nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 6.0.0 (https://github.com/llvm-mirror/clang.git 0e746072ed897a85b4f533ab050b9f506941a097) (git@version.aalto.fi:platsec/pointer-authentication/pa-llvm.git c0872c7748aeb787574a0d71b195ec301f29f0e4)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!4, !4, i64 0}
!4 = !{!"any pointer", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"a_struct", !9, i64 0, !9, i64 4}
!9 = !{!"int", !5, i64 0}
!10 = !{!11, !12, i64 0}
!11 = !{!"b_struct", !12, i64 0, !12, i64 8, !12, i64 16}
!12 = !{!"long", !5, i64 0}
!13 = !{!12, !12, i64 0}
!14 = !{!9, !9, i64 0}
