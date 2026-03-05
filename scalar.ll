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
  %0 = alloca i32, align 4
  store i32 10, ptr %0, align 4
  %1 = alloca i32, align 4
  store i32 5, ptr %1, align 4
  %2 = load i32, ptr %0, align 4
  %3 = load i32, ptr %1, align 4
  %4 = mul i32 %3, 2
  %5 = add i32 %2, %4
  %6 = alloca i32, align 4
  store i32 %5, ptr %6, align 4
  %7 = load i32, ptr %0, align 4
  %8 = call i32 (ptr, ...) @printf(ptr @0, i32 %7)
  %9 = load i32, ptr %1, align 4
  %10 = call i32 (ptr, ...) @printf(ptr @1, i32 %9)
  %11 = load i32, ptr %6, align 4
  %12 = call i32 (ptr, ...) @printf(ptr @2, i32 %11)
  %13 = alloca double, align 8
  store double 3.500000e+00, ptr %13, align 8
  %14 = alloca double, align 8
  store double 1.500000e+00, ptr %14, align 8
  %15 = load double, ptr %13, align 8
  %16 = load double, ptr %14, align 8
  %17 = fmul double %16, 2.000000e+00
  %18 = fadd double %15, %17
  %19 = alloca double, align 8
  store double %18, ptr %19, align 8
  %20 = load double, ptr %13, align 8
  %21 = call i32 (ptr, ...) @printf(ptr @3, double %20)
  %22 = load double, ptr %14, align 8
  %23 = call i32 (ptr, ...) @printf(ptr @4, double %22)
  %24 = load double, ptr %19, align 8
  %25 = call i32 (ptr, ...) @printf(ptr @5, double %24)
  %26 = alloca ptr, align 8
  store ptr @6, ptr %26, align 8
  %27 = load ptr, ptr %26, align 8
  %28 = call i32 (ptr, ...) @printf(ptr @7, ptr %27)
  %29 = load i32, ptr %0, align 4
  %30 = add i32 %29, 100
  store i32 %30, ptr %0, align 4
  %31 = load i32, ptr %0, align 4
  %32 = call i32 (ptr, ...) @printf(ptr @8, i32 %31)
  %33 = alloca i32, i32 4, align 4
  %34 = getelementptr i32, ptr %33, i32 0
  store i32 10, ptr %34, align 4
  %35 = getelementptr i32, ptr %33, i32 1
  store i32 20, ptr %35, align 4
  %36 = getelementptr i32, ptr %33, i32 2
  store i32 30, ptr %36, align 4
  %37 = getelementptr i32, ptr %33, i32 3
  store i32 40, ptr %37, align 4
  %38 = alloca i32, align 4
  store i32 0, ptr %38, align 4
  br label %loop_cond

loop_cond:                                        ; preds = %loop_body, %entry
  %39 = load i32, ptr %38, align 4
  %40 = icmp slt i32 %39, 4
  br i1 %40, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_cond
  %41 = load i32, ptr %38, align 4
  %42 = getelementptr i32, ptr %33, i32 %41
  %43 = load i32, ptr %42, align 4
  %44 = call i32 (ptr, ...) @printf(ptr @9, i32 %43)
  %45 = load i32, ptr %38, align 4
  %46 = add i32 %45, 1
  store i32 %46, ptr %38, align 4
  br label %loop_cond

loop_end:                                         ; preds = %loop_cond
  %47 = alloca i32, i32 3, align 4
  %48 = getelementptr i32, ptr %47, i32 0
  store i32 3, ptr %48, align 4
  %49 = getelementptr i32, ptr %47, i32 1
  store i32 12, ptr %49, align 4
  %50 = getelementptr i32, ptr %47, i32 2
  store i32 5, ptr %50, align 4
  %51 = alloca i32, align 4
  store i32 0, ptr %51, align 4
  br label %loop_cond1

loop_cond1:                                       ; preds = %loop_body2, %loop_end
  %52 = load i32, ptr %51, align 4
  %53 = icmp slt i32 %52, 3
  br i1 %53, label %loop_body2, label %loop_end3

loop_body2:                                       ; preds = %loop_cond1
  %54 = load i32, ptr %51, align 4
  %55 = getelementptr i32, ptr %47, i32 %54
  %56 = load i32, ptr %55, align 4
  %57 = call i32 (ptr, ...) @printf(ptr @10, i32 %56)
  %58 = load i32, ptr %51, align 4
  %59 = add i32 %58, 1
  store i32 %59, ptr %51, align 4
  br label %loop_cond1

loop_end3:                                        ; preds = %loop_cond1
  %60 = alloca i32, align 4
  store i32 48, ptr %60, align 4
  %61 = load i32, ptr %60, align 4
  %62 = call i32 (ptr, ...) @printf(ptr @11, i32 %61)
  %63 = alloca i32, align 4
  store i32 4, ptr %63, align 4
  %64 = load i32, ptr %63, align 4
  %65 = alloca i32, align 4
  store i32 0, ptr %65, align 4
  br label %loop_cond4

loop_cond4:                                       ; preds = %loop_body5, %loop_end3
  %66 = load i32, ptr %65, align 4
  %67 = icmp slt i32 %66, %64
  br i1 %67, label %loop_body5, label %loop_end6

loop_body5:                                       ; preds = %loop_cond4
  %68 = load i32, ptr %65, align 4
  %69 = call i32 (ptr, ...) @printf(ptr @12, i32 %68)
  %70 = load i32, ptr %65, align 4
  %71 = add i32 %70, 1
  store i32 %71, ptr %65, align 4
  br label %loop_cond4

loop_end6:                                        ; preds = %loop_cond4
  ret i32 0
}
