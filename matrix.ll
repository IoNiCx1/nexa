; ModuleID = 'nexa'
source_filename = "nexa"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@2 = private unnamed_addr constant [3 x i8] c"[ \00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@4 = private unnamed_addr constant [3 x i8] c"]\0A\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 77, ptr %x, align 4
  %a = alloca [9 x i32], align 4
  store [9 x i32] zeroinitializer, ptr %a, align 4
  %0 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 0
  store i32 1, ptr %0, align 4
  %1 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 1
  store i32 2, ptr %1, align 4
  %2 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 2
  store i32 3, ptr %2, align 4
  %3 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 3
  store i32 4, ptr %3, align 4
  %4 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 4
  store i32 5, ptr %4, align 4
  %5 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 5
  store i32 90, ptr %5, align 4
  %6 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 6
  store i32 767, ptr %6, align 4
  %7 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 7
  store i32 67, ptr %7, align 4
  %8 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 8
  store i32 -12, ptr %8, align 4
  %9 = load i32, ptr %x, align 4
  %10 = call i32 (ptr, ...) @printf(ptr @1, i32 %9)
  %11 = call i32 (ptr, ...) @printf(ptr @2)
  %12 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 0
  %13 = load i32, ptr %12, align 4
  %14 = call i32 (ptr, ...) @printf(ptr @3, i32 %13)
  %15 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 1
  %16 = load i32, ptr %15, align 4
  %17 = call i32 (ptr, ...) @printf(ptr @3, i32 %16)
  %18 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 2
  %19 = load i32, ptr %18, align 4
  %20 = call i32 (ptr, ...) @printf(ptr @3, i32 %19)
  %21 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 3
  %22 = load i32, ptr %21, align 4
  %23 = call i32 (ptr, ...) @printf(ptr @3, i32 %22)
  %24 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 4
  %25 = load i32, ptr %24, align 4
  %26 = call i32 (ptr, ...) @printf(ptr @3, i32 %25)
  %27 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 5
  %28 = load i32, ptr %27, align 4
  %29 = call i32 (ptr, ...) @printf(ptr @3, i32 %28)
  %30 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 6
  %31 = load i32, ptr %30, align 4
  %32 = call i32 (ptr, ...) @printf(ptr @3, i32 %31)
  %33 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 7
  %34 = load i32, ptr %33, align 4
  %35 = call i32 (ptr, ...) @printf(ptr @3, i32 %34)
  %36 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 8
  %37 = load i32, ptr %36, align 4
  %38 = call i32 (ptr, ...) @printf(ptr @3, i32 %37)
  %39 = call i32 (ptr, ...) @printf(ptr @4)
  %40 = call i32 (ptr, ...) @printf(ptr @5, ptr @0)
  ret i32 0
}

declare i32 @printf(ptr, ...)
