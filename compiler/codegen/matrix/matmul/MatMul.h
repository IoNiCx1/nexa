#pragma once
#include "../../LLVMContext.h"

/*
  Lowers matrix multiplication into LLVM IR
  C = A (MxK) * B (KxN)
*/
class MatMul {
public:
    explicit MatMul(LLVMState& state);

    llvm::Value* generate(
        llvm::Value* A,
        llvm::Value* B,
        int M,
        int K,
        int N
    );

private:
    LLVMState& llvm;
};
