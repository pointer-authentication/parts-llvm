; Check metadata for data pointer authentication is added
; RUN: opt -load PartsOpt.so -parts-dpi -parts-opt-dpi -S < %s | FileCheck %s

define i8* @simple_pointer() {
entry:
  %data = alloca i8*, align 8
  %call = call i8* @my_simple_alloc(i64 4096)
  store i8* %call, i8** %data, align 8
; CHECK: store i8* %call, i8** %data, align 8, !PartsTypeMetadata !0
  %0 = load i8*, i8** %data, align 8
; CHECK: %0 = load i8*, i8** %data, align 8, !PartsTypeMetadata !0
  ret i8* %0
}

declare i8* @my_simple_alloc(i64)

; CHECK: !0 = !{!"PartsTypeMetadata", i64 {{-?[0-9]+}}, i1 true, i1 true, i1 true, i1 false, i1 false}
