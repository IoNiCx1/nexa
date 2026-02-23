; ModuleID = 'nexa_module'
source_filename = "nexa_module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@3 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@4 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
@6 = private unnamed_addr constant [17 x i8] c"Nexa is working!\00", align 1
@7 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@8 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 10, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 5, ptr %b, align 4
  %precedence1 = alloca i32, align 4
  store i32 20, ptr %precedence1, align 4
  %precedence2 = alloca i32, align 4
  store i32 30, ptr %precedence2, align 4
  %0 = load i32, ptr %precedence1, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @0, i32 %0)
  %2 = load i32, ptr %precedence2, align 4
  %3 = call i32 (ptr, ...) @printf(ptr @1, i32 %2)
  %neg = alloca i32, align 4
  %4 = load i32, ptr %a, align 4
  %5 = sub i32 0, %4
  store i32 %5, ptr %neg, align 4
  %6 = load i32, ptr %neg, align 4
  %7 = call i32 (ptr, ...) @printf(ptr @2, i32 %6)
  %x = alloca double, align 8
  store double 3.500000e+00, ptr %x, align 8
  %y = alloca double, align 8
  store double 2.000000e+00, ptr %y, align 8
  %dsum = alloca double, align 8
  %8 = load double, ptr %x, align 8
  %9 = load double, ptr %y, align 8
  %10 = fadd double %8, %9
  store double %10, ptr %dsum, align 8
  %dmul = alloca double, align 8
  %11 = load double, ptr %x, align 8
  %12 = load double, ptr %y, align 8
  %13 = fmul double %11, %12
  store double %13, ptr %dmul, align 8
  %14 = load double, ptr %dsum, align 8
  %15 = call i32 (ptr, ...) @printf(ptr @3, double %14)
  %16 = load double, ptr %dmul, align 8
  %17 = call i32 (ptr, ...) @printf(ptr @4, double %16)
  %mixed = alloca double, align 8
  %18 = load i32, ptr %a, align 4
  %19 = load double, ptr %x, align 8
  %20 = sitofp i32 %18 to double
  %21 = fadd double %20, %19
  store double %21, ptr %mixed, align 8
  %22 = load double, ptr %mixed, align 8
  %23 = call i32 (ptr, ...) @printf(ptr @5, double %22)
  %msg = alloca ptr, align 8
  store ptr @6, ptr %msg, align 8
  %24 = load ptr, ptr %msg, align 8
  %25 = call i32 (ptr, ...) @printf(ptr @7, ptr %24)
  %complex = alloca i32, align 4
  %26 = load i32, ptr %a, align 4
  %27 = load i32, ptr %b, align 4
  %28 = add i32 %26, %27
  %29 = mul i32 %28, 2
  %30 = load i32, ptr %b, align 4
  %31 = sdiv i32 %30, 5
  %32 = sub i32 %29, %31
  %33 = add i32 %32, 3
  store i32 %33, ptr %complex, align 4
  %34 = load i32, ptr %complex, align 4
  %35 = call i32 (ptr, ...) @printf(ptr @8, i32 %34)
  ret i32 0
}
