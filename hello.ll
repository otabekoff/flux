; ModuleID = 'hello'
source_filename = "hello"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@str = private unnamed_addr constant [13 x i8] c"Hello, Flux!\00", align 1

define void @main() {
entry:
  %result = alloca i32, align 4
  %counter = alloca i32, align 4
  %message = alloca ptr, align 8
  store ptr @str, ptr %message, align 8
  store i32 0, ptr %counter, align 4
  %counter1 = load i32, ptr %counter, align 4
  %sext = sext i32 %counter1 to i64
  %addtmp = add i64 %sext, 1
  store i64 %addtmp, ptr %counter, align 8
  ret void
}

define internal i32 @add(i32 %a, i32 %b) {
entry:
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a3 = load i32, ptr %a1, align 4
  %b4 = load i32, ptr %b2, align 4
  %addtmp = add i32 %a3, %b4
  ret i32 %addtmp
}
