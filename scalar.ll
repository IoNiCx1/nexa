; ModuleID = 'nexa_module'
source_filename = "nexa_module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@6 = private unnamed_addr constant [22 x i8] c"Nexa Compiler Working\00", align 1
@7 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@8 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@9 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@10 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@11 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@12 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 10, ptr %x, align 4
  %y = alloca i32, align 4
  store i32 5, ptr %y, align 4
  %0 = load i32, ptr %x, align 4
  %1 = load i32, ptr %y, align 4
  %2 = mul i32 %1, 2
  %3 = add i32 %0, %2
  %z = alloca i32, align 4
  store i32 %3, ptr %z, align 4
  %4 = load i32, ptr %x, align 4
  %5 = call i32 (ptr, ...) @printf(ptr @0, i32 %4)
  %6 = load i32, ptr %y, align 4
  %7 = call i32 (ptr, ...) @printf(ptr @1, i32 %6)
  %8 = load i32, ptr %z, align 4
  %9 = call i32 (ptr, ...) @printf(ptr @2, i32 %8)
  %a = alloca double, align 8
  store double 3.500000e+00, ptr %a, align 8
  %b = alloca double, align 8
  store double 1.500000e+00, ptr %b, align 8
  %10 = load double, ptr %a, align 8
  %11 = load double, ptr %b, align 8
  %12 = fmul double %11, 2.000000e+00
  %13 = fadd double %10, %12
  %c = alloca double, align 8
  store double %13, ptr %c, align 8
  %14 = load double, ptr %a, align 8
  %15 = call i32 (ptr, ...) @printf(ptr @3, double %14)
  %16 = load double, ptr %b, align 8
  %17 = call i32 (ptr, ...) @printf(ptr @4, double %16)
  %18 = load double, ptr %c, align 8
  %19 = call i32 (ptr, ...) @printf(ptr @5, double %18)
  %msg = alloca ptr, align 8
  store ptr @6, ptr %msg, align 8
  %20 = load ptr, ptr %msg, align 8
  %21 = call i32 (ptr, ...) @printf(ptr @7, ptr %20)
  %22 = load i32, ptr %x, align 4
  %23 = add i32 %22, 100
  store i32 %23, ptr %x, align 4
  %24 = load i32, ptr %x, align 4
  %25 = call i32 (ptr, ...) @printf(ptr @8, i32 %24)
  %26 = alloca [4 x i32], align 4
  %27 = getelementptr [4 x i32], ptr %26, i32 0, i32 0
  store i32 10, ptr %27, align 4
  %28 = getelementptr [4 x i32], ptr %26, i32 0, i32 1
  store i32 20, ptr %28, align 4
  %29 = getelementptr [4 x i32], ptr %26, i32 0, i32 2
  store i32 30, ptr %29, align 4
  %30 = getelementptr [4 x i32], ptr %26, i32 0, i32 3
  store i32 40, ptr %30, align 4
  %31 = getelementptr [4 x i32], ptr %26, i32 0, i32 0
  %arr = alloca ptr, align 8
  store ptr %31, ptr %arr, align 8
  %i = alloca i32, align 4
  store i32 0, ptr %i, align 4
  br label %loop_cond

loop_cond:                                        ; preds = %loop_body, %entry
  %32 = load i32, ptr %i, align 4
  %33 = icmp slt i32 %32, 4
  br i1 %33, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_cond
  %34 = load ptr, ptr %arr, align 8
  %35 = load i32, ptr %i, align 4
  %36 = getelementptr i32, ptr %34, i32 %35
  %37 = load i32, ptr %36, align 4
  %38 = call i32 (ptr, ...) @printf(ptr @9, i32 %37)
  %39 = load i32, ptr %i, align 4
  %40 = add i32 %39, 1
  store i32 %40, ptr %i, align 4
  br label %loop_cond

loop_end:                                         ; preds = %loop_cond
  %41 = alloca [3 x i32], align 4
  %42 = getelementptr [3 x i32], ptr %41, i32 0, i32 0
  store i32 3, ptr %42, align 4
  %43 = getelementptr [3 x i32], ptr %41, i32 0, i32 1
  store i32 12, ptr %43, align 4
  %44 = getelementptr [3 x i32], ptr %41, i32 0, i32 2
  store i32 5, ptr %44, align 4
  %45 = getelementptr [3 x i32], ptr %41, i32 0, i32 0
  %arr2 = alloca ptr, align 8
  store ptr %45, ptr %arr2, align 8
  %k = alloca i32, align 4
  store i32 0, ptr %k, align 4
  br label %loop_cond1

loop_cond1:                                       ; preds = %loop_body2, %loop_end
  %46 = load i32, ptr %k, align 4
  %47 = icmp slt i32 %46, 3
  br i1 %47, label %loop_body2, label %loop_end3

loop_body2:                                       ; preds = %loop_cond1
  %48 = load ptr, ptr %arr2, align 8
  %49 = load i32, ptr %k, align 4
  %50 = getelementptr i32, ptr %48, i32 %49
  %51 = load i32, ptr %50, align 4
  %52 = call i32 (ptr, ...) @printf(ptr @10, i32 %51)
  %53 = load i32, ptr %k, align 4
  %54 = add i32 %53, 1
  store i32 %54, ptr %k, align 4
  br label %loop_cond1

loop_end3:                                        ; preds = %loop_cond1
  %complex = alloca i32, align 4
  store i32 48, ptr %complex, align 4
  %55 = load i32, ptr %complex, align 4
  %56 = call i32 (ptr, ...) @printf(ptr @11, i32 %55)
  %count = alloca i32, align 4
  store i32 4, ptr %count, align 4
  %57 = load i32, ptr %count, align 4
  %m = alloca i32, align 4
  store i32 0, ptr %m, align 4
  br label %loop_cond4

loop_cond4:                                       ; preds = %loop_body5, %loop_end3
  %58 = load i32, ptr %m, align 4
  %59 = icmp slt i32 %58, %57
  br i1 %59, label %loop_body5, label %loop_end6

loop_body5:                                       ; preds = %loop_cond4
  %60 = load i32, ptr %m, align 4
  %61 = call i32 (ptr, ...) @printf(ptr @12, i32 %60)
  %62 = load i32, ptr %m, align 4
  %63 = add i32 %62, 1
  store i32 %63, ptr %m, align 4
  br label %loop_cond4

loop_end6:                                        ; preds = %loop_cond4
  ret i32 0
}
