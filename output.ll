; ModuleID = 'nexa_module'
source_filename = "nexa_module"

@0 = private unnamed_addr constant [6 x i8] c"Omkar\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

declare i32 @printf(ptr, ...)

declare ptr @malloc(i64)

define i32 @main() {
entry:
  %0 = alloca i32, align 4
  store i32 0, ptr %0, align 4
  br label %loop_cond

loop_cond:                                        ; preds = %loop_body, %entry
  %1 = load i32, ptr %0, align 4
  %2 = icmp slt i32 %1, 5
  br i1 %2, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_cond
  %3 = call i32 (ptr, ...) @printf(ptr @1, ptr @0)
  %4 = add i32 %1, 1
  store i32 %4, ptr %0, align 4
  br label %loop_cond

loop_end:                                         ; preds = %loop_cond
  ret i32 0
}
