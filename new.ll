; ModuleID = 'nexa_module'
source_filename = "nexa_module"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @mul(i32 %0, i32 %1) {
entry:
  %a = alloca i32, align 4
  store i32 %0, ptr %a, align 4
  %b = alloca i32, align 4
  store i32 %1, ptr %b, align 4
  %b1 = load i32, ptr %b, align 4
  %a2 = alloca i32, align 4
  store i32 0, ptr %a2, align 4
  br label %loop_cond

loop_cond:                                        ; preds = %loop_body, %entry
  %2 = load i32, ptr %a2, align 4
  %3 = icmp slt i32 %2, %b1
  br i1 %3, label %loop_body, label %loop_end

loop_body:                                        ; preds = %loop_cond
  %4 = call i32 (ptr, ...) @printf(ptr @1, ptr @0)
  %5 = load i32, ptr %a2, align 4
  %6 = add i32 %5, 1
  store i32 %6, ptr %a2, align 4
  br label %loop_cond

loop_end:                                         ; preds = %loop_cond
  ret i32 0
}

define i32 @main() {
entry:
  %0 = call i32 @mul(i32 2, i32 3)
  %1 = call i32 (ptr, ...) @printf(ptr @2, i32 %0)
  ret i32 0
}
