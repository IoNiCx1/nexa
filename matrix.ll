; ModuleID = 'nexa'
source_filename = "nexa"

@0 = private unnamed_addr constant [12 x i8] c"hello world\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@2 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@4 = private unnamed_addr constant [3 x i8] c"[ \00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@6 = private unnamed_addr constant [3 x i8] c"]\0A\00", align 1
@7 = private unnamed_addr constant [3 x i8] c"[ \00", align 1
@8 = private unnamed_addr constant [4 x i8] c"%d \00", align 1
@9 = private unnamed_addr constant [3 x i8] c"]\0A\00", align 1

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 77, ptr %x, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @1, ptr @0)
  %a = alloca [9 x i32], align 4
  %1 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 0
  store i32 1, ptr %1, align 4
  %2 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 1
  store i32 2, ptr %2, align 4
  %3 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 2
  store i32 3, ptr %3, align 4
  %4 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 3
  store i32 4, ptr %4, align 4
  %5 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 4
  store i32 5, ptr %5, align 4
  %6 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 5
  store i32 90, ptr %6, align 4
  %7 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 6
  store i32 767, ptr %7, align 4
  %8 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 7
  store i32 67, ptr %8, align 4
  %9 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 8
  store i32 -12, ptr %9, align 4
  %c = alloca [1 x i32], align 4
  %c1 = alloca [4 x i32], align 4
  %10 = getelementptr inbounds [4 x i32], ptr %c1, i32 0, i32 0
  %x2 = load i32, ptr %x, align 4
  store i32 %x2, ptr %10, align 4
  %11 = getelementptr inbounds [4 x i32], ptr %c1, i32 0, i32 1
  store i32 10, ptr %11, align 4
  %12 = getelementptr inbounds [4 x i32], ptr %c1, i32 0, i32 2
  store i32 20, ptr %12, align 4
  %13 = getelementptr inbounds [4 x i32], ptr %c1, i32 0, i32 3
  store i32 30, ptr %13, align 4
  %s = alloca [1 x i32], align 4
  %14 = getelementptr inbounds [1 x i32], ptr %s, i32 0, i32 0
  store ptr @2, ptr %14, align 8
  %x3 = load i32, ptr %x, align 4
  %15 = call i32 (ptr, ...) @printf(ptr @3, i32 %x3)
  %16 = call i32 (ptr, ...) @printf(ptr @4)
  %17 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 0
  %18 = load i32, ptr %17, align 4
  %19 = call i32 (ptr, ...) @printf(ptr @5, i32 %18)
  %20 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 1
  %21 = load i32, ptr %20, align 4
  %22 = call i32 (ptr, ...) @printf(ptr @5, i32 %21)
  %23 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 2
  %24 = load i32, ptr %23, align 4
  %25 = call i32 (ptr, ...) @printf(ptr @5, i32 %24)
  %26 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 3
  %27 = load i32, ptr %26, align 4
  %28 = call i32 (ptr, ...) @printf(ptr @5, i32 %27)
  %29 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 4
  %30 = load i32, ptr %29, align 4
  %31 = call i32 (ptr, ...) @printf(ptr @5, i32 %30)
  %32 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 5
  %33 = load i32, ptr %32, align 4
  %34 = call i32 (ptr, ...) @printf(ptr @5, i32 %33)
  %35 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 6
  %36 = load i32, ptr %35, align 4
  %37 = call i32 (ptr, ...) @printf(ptr @5, i32 %36)
  %38 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 7
  %39 = load i32, ptr %38, align 4
  %40 = call i32 (ptr, ...) @printf(ptr @5, i32 %39)
  %41 = getelementptr inbounds [9 x i32], ptr %a, i32 0, i32 8
  %42 = load i32, ptr %41, align 4
  %43 = call i32 (ptr, ...) @printf(ptr @5, i32 %42)
  %44 = call i32 (ptr, ...) @printf(ptr @6)
  %45 = call i32 (ptr, ...) @printf(ptr @7)
  %46 = getelementptr inbounds [1 x i32], ptr %s, i32 0, i32 0
  %47 = load i32, ptr %46, align 4
  %48 = call i32 (ptr, ...) @printf(ptr @8, i32 %47)
  %49 = call i32 (ptr, ...) @printf(ptr @9)
  ret i32 0
}

declare i32 @printf(ptr, ...)
