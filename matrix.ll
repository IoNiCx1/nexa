; ModuleID = 'nexa'
source_filename = "nexa"

@0 = private unnamed_addr constant [12 x i8] c"hello world\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%s\0A\00", align 1

define i32 @main() {
entry:
  %alpha = alloca i32, align 4
  store i32 77, ptr %alpha, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @1, ptr @0)
  ret i32 77
}

declare i32 @printf(ptr, ...)
