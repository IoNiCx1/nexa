; ModuleID = 'nexa'
source_filename = "nexa"

@0 = private unnamed_addr constant [12 x i8] c"hello world\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@2 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 77, ptr %x, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @1, ptr @0)
  %a = alloca [5 x i32], align 4
  %1 = getelementptr inbounds [5 x i32], ptr %a, i32 0, i32 0
  store i32 1, ptr %1, align 4
  %2 = getelementptr inbounds [5 x i32], ptr %a, i32 0, i32 1
  store i32 2, ptr %2, align 4
  %3 = getelementptr inbounds [5 x i32], ptr %a, i32 0, i32 2
  store i32 3, ptr %3, align 4
  %4 = getelementptr inbounds [5 x i32], ptr %a, i32 0, i32 3
  store i32 4, ptr %4, align 4
  %5 = getelementptr inbounds [5 x i32], ptr %a, i32 0, i32 4
  store i32 5, ptr %5, align 4
  %c = alloca [1 x i32], align 4
  %6 = getelementptr inbounds [4 x i32], ptr %c, i32 0, i32 0
  %x1 = load i32, ptr %x, align 4
  store i32 %x1, ptr %6, align 4
  %7 = getelementptr inbounds [4 x i32], ptr %c, i32 0, i32 1
  store i32 10, ptr %7, align 4
  %8 = getelementptr inbounds [4 x i32], ptr %c, i32 0, i32 2
  store i32 20, ptr %8, align 4
  %9 = getelementptr inbounds [4 x i32], ptr %c, i32 0, i32 3
  store i32 30, ptr %9, align 4
  %s = alloca [1 x i32], align 4
  %10 = getelementptr inbounds [1 x i32], ptr %s, i32 0, i32 0
  store ptr @2, ptr %10, align 8
  %x2 = load i32, ptr %x, align 4
  %11 = call i32 (ptr, ...) @printf(ptr @3, i32 %x2)
  %12 = call i32 (ptr, ...) @printf(ptr @4, ptr %a)
  ret i32 0
}

declare i32 @printf(ptr, ...)
