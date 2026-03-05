; ModuleID = 'nexa_module'
source_filename = "nexa_module"

@0 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %0 = alloca i1, align 1
  store i1 false, ptr %0, align 1
  %1 = load i1, ptr %0, align 1
  br i1 %1, label %then, label %else

then:                                             ; preds = %entry
  %2 = call i32 (ptr, ...) @printf(ptr @0, i32 1)
  br label %ifcont

else:                                             ; preds = %entry
  %3 = call i32 (ptr, ...) @printf(ptr @1, i32 0)
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  ret i32 0
}
