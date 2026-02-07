; ModuleID = 'nexa'
source_filename = "nexa"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 70, ptr %a, align 4
  %0 = load i32, ptr %a, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @0, i32 %0)
  %b = alloca i32, align 4
  store i32 90, ptr %b, align 4
  %2 = load i32, ptr %b, align 4
  %3 = call i32 (ptr, ...) @printf(ptr @1, i32 %2)
  %c = alloca i32, align 4
  store i32 -5, ptr %c, align 4
  %4 = load i32, ptr %c, align 4
  %5 = call i32 (ptr, ...) @printf(ptr @2, i32 %4)
  ret i32 0
}

declare i32 @printf(ptr, ...)
