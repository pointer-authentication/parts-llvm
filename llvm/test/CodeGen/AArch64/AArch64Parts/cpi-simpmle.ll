; RUN: llc -verify-machineinstrs -mtriple=aarch64-none-linux-gnu -mattr=v8.3a -parts-fecfi < %s | FileCheck %s
;
; Simple tests for code pointer integrity, based on:
;
; Seems to be somemthing still wrong with this test, should be okay
;
;void printer1();
;void printer2();
;
;void gimme_func_ptr(void (*f)(void));
;void gimme_func_ptr2(void (*f1)(void), void (*f2)(void));
;
;void (*func1)(void) = printer1;
;void (*func2)(void) = printer2;
;
;void printer1() {
;   printf("%s\n", __FUNCTION__);
;}
;
;void printer2() {
;    printf("%s\n", __FUNCTION__);
;}
;
;void gimme_func_ptr(void (*f)(void))
;{
;    f();
;}
;
;void gimme_func_ptr2(void (*f1)(void), void (*f2)(void))
;{
;    f1();
;    f2();
;}
;
;int main(int argc, char **argv)
;{
;    (void) argc;
;    (void) argv;
;
;    func1();
;    func2();
;
;    gimme_func_ptr(func1);
;    gimme_func_ptr(func1);
;
;    func1 = printer2;
;
;    gimme_func_ptr(func1);
;    gimme_func_ptr(func2);
;
;    gimme_func_ptr2(func1, func2);
;

;    return 0;
;}
;

target triple = "aarch64"

@func1 = hidden global void ()* @printer1, align 8
@.str = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@__FUNCTION__.printer1 = private unnamed_addr constant [9 x i8] c"printer1\00", align 1
@func2 = hidden global void ()* @printer2, align 8
@__FUNCTION__.printer2 = private unnamed_addr constant [9 x i8] c"printer2\00", align 1

declare i32 @printf(i8*, ...) #1

define hidden void @printer1() #0 {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @__FUNCTION__.printer1, i32 0, i32 0))
  ret void
}

define hidden void @printer2() #0 {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @__FUNCTION__.printer2, i32 0, i32 0))
  ret void
}

define hidden void @gimme_func_ptr(void ()* %f) #0 {
entry:
  %f.addr = alloca void ()*, align 8
  store void ()* %f, void ()** %f.addr, align 8
  %0 = load void ()*, void ()** %f.addr, align 8
  %1 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %0, i64 -8151429658862389052)
  call void %1()
  ; CHECK: blraa
  ret void
}

define hidden void @gimme_func_ptr2(void ()* %f1, void ()* %f2) #0 {
entry:
  %f1.addr = alloca void ()*, align 8
  %f2.addr = alloca void ()*, align 8
  store void ()* %f1, void ()** %f1.addr, align 8
  store void ()* %f2, void ()** %f2.addr, align 8
  %0 = load void ()*, void ()** %f1.addr, align 8
  %1 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %0, i64 -8151429658862389052)
  call void %1()
  ; CHECK: blraa
  %2 = load void ()*, void ()** %f2.addr, align 8
  %3 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %2, i64 -8151429658862389052)
  call void %3()
  ; CHECK: blraa
  ret void
}

define hidden i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  store i32 0, i32* %retval, align 4
  store i32 %argc, i32* %argc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  %0 = load i32, i32* %argc.addr, align 4
  %1 = load i8**, i8*** %argv.addr, align 8

  %2 = load void ()*, void ()** @func1, align 8
  %3 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %2, i64 -8151429658862389052)
  ; CHECK-NO: autia
  call void %3()
  ; CHECK: blraa

  %4 = load void ()*, void ()** @func2, align 8
  %5 = call void ()* @llvm.pa.autcall.p0f_isVoidf(void ()* %4, i64 -8151429658862389052)
  ; CHECK-NO: autia
  call void %5()
  ; CHECK: blraa

  %6 = load void ()*, void ()** @func1, align 8
  ; CHECK-NO: autia
  call void @gimme_func_ptr(void ()* %6)

  %7 = load void ()*, void ()** @func1, align 8
  ; CHECK-NO: autia
  call void @gimme_func_ptr(void ()* %7)

  %8 = call void ()* @llvm.pa.pacia.p0f_isVoidf(void ()* @printer2, i64 -8151429658862389052)
  ; CHECK-NO: autia
  store void ()* %8, void ()** @func1, align 8

  %9 = load void ()*, void ()** @func1, align 8
  ; CHECK-NO: autia
  call void @gimme_func_ptr(void ()* %9)
  ; CHECK: bl

  %10 = load void ()*, void ()** @func2, align 8
  ; CHECK-NO: autia
  call void @gimme_func_ptr(void ()* %10)
  ; CHECK: bl

  %11 = load void ()*, void ()** @func1, align 8
  ; CHECK-NO: autia
  %12 = load void ()*, void ()** @func2, align 8
  ; CHECK-NO: autia
  call void @gimme_func_ptr2(void ()* %11, void ()* %12)
  ret i32 0
}

; Function Attrs: nounwind readnone
declare void ()* @llvm.pa.pacia.p0f_isVoidf(void ()*, i64) #4

; Function Attrs: nounwind readnone
declare void ()* @llvm.pa.autcall.p0f_isVoidf(void ()*, i64) #4

attributes #0 = { noinline nounwind optnone }
; attributes #0 = { noinline nounwind optnone "no-frame-pointer-elim-non-leaf" }
; attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+neon" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone }
