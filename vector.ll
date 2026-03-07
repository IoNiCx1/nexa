; ModuleID = 'nexa_module'
source_filename = "nexa_module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @add(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %a1 = load i32, ptr %a, align 4
  %b2 = load i32, ptr %b, align 4
  %2 = add i32 %a1, %b2
  ret i32 %2
}

define i32 @main() {
entry:
  %0 = call i32 @add(i32 2, i32 3)
  %1 = call i32 (ptr, ...) @printf(ptr @0, i32 %0)
  %2 = call i32 @add(i32 5, i32 6)
  %3 = call i32 (ptr, ...) @printf(ptr @1, i32 %2)
  ret i32 0
}
