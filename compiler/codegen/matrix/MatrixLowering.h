#pragma once
#include "../LLVMContext.h"
#include "../../ast/Ast.h"

/*
  Lowers vector/matrix operations into LLVM IR loops
*/
class MatrixLowering {
public:
    explicit MatrixLowering(LLVMState& state);

    // Allocate matrix on stack
    llvm::Value* allocateMatrix(const TypeSpec& type, const std::string& name);

    // Store scalar/vector values into matrix memory
    void storeVector(
        llvm::Value* matrixPtr,
        const std::vector<llvm::Value*>& values
    );

    // Dot product
    llvm::Value* dot(
        llvm::Value* A,
        llvm::Value* B,
        int N
    );

private:
    LLVMState& llvm;
};
