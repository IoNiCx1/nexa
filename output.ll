; ModuleID = 'nexa_module'
source_filename = "nexa_module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

define i32 @main() {
entry:
  %0 = alloca i32, i32 4, align 4
  %1 = getelementptr i32, ptr %0, i32 0
  store i32 10, ptr %1, align 4
  %2 = getelementptr i32, ptr %0, i32 1
  store i32 20, ptr %2, align 4
  %3 = getelementptr i32, ptr %0, i32 2
  store i32 30, ptr %3, align 4
  %4 = getelementptr i32, ptr %0, i32 3
  store i32 40, ptr %4, align 4
  %5 = alloca i32, align 4
  store i32 0, ptr %5, align 4
  br label %loop_cond

loop_cond:                                        ; preds = %loop_body, %entry
  %6 = load i32, ptr %5, align 4
  %7 = icmp slt i32 %6, 4
  br i1 %7, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_cond
  %8 = load i32, ptr %5, align 4
  %9 = getelementptr i32, ptr %0, i32 %8
  %10 = load i32, ptr %9, align 4
  %11 = call i32 (ptr, ...) @printf(ptr @0, i32 %10)
  %12 = add i32 %6, 1
  store i32 %12, ptr %5, align 4
  br label %loop_cond

loop_end:                                         ; preds = %loop_cond
  ret i32 0
}
