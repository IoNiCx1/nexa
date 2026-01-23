#include "MatrixLowering.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

MatrixLowering::MatrixLowering(LLVMState& state) : llvm(state) {}

llvm::Value* MatrixLowering::allocateMatrix(
    const TypeSpec& type,
    const std::string& name
) {
    using namespace llvm;

    int total = type.rows * type.cols;
    Type* elemTy = Type::getDoubleTy(llvm.context);

    ArrayType* arrTy = ArrayType::get(elemTy, total);
    return llvm.builder.CreateAlloca(arrTy, nullptr, name);
}

void MatrixLowering::storeVector(
    llvm::Value* matrixPtr,
    const std::vector<llvm::Value*>& values
) {
    using namespace llvm;

    for (size_t i = 0; i < values.size(); ++i) {
        Value* idx[] = {
            ConstantInt::get(Type::getInt32Ty(llvm.context), 0),
            ConstantInt::get(Type::getInt32Ty(llvm.context), i)
        };

        Value* elemPtr =
            llvm.builder.CreateGEP(matrixPtr, idx);

        llvm.builder.CreateStore(values[i], elemPtr);
    }
}

llvm::Value* MatrixLowering::dot(
    llvm::Value* A,
    llvm::Value* B,
    int N
) {
    using namespace llvm;

    Function* func = llvm.builder.GetInsertBlock()->getParent();

    BasicBlock* loopBB = BasicBlock::Create(
        llvm.context, "dot.loop", func);
    BasicBlock* afterBB = BasicBlock::Create(
        llvm.context, "dot.after", func);

    // sum = 0
    AllocaInst* sum =
        llvm.builder.CreateAlloca(
            Type::getDoubleTy(llvm.context), nullptr, "sum");

    llvm.builder.CreateStore(
        ConstantFP::get(Type::getDoubleTy(llvm.context), 0.0), sum);

    // i = 0
    AllocaInst* iVar =
        llvm.builder.CreateAlloca(
            Type::getInt32Ty(llvm.context), nullptr, "i");

    llvm.builder.CreateStore(
        ConstantInt::get(Type::getInt32Ty(llvm.context), 0), iVar);

    llvm.builder.CreateBr(loopBB);
    llvm.builder.SetInsertPoint(loopBB);

    // load i
    Value* iVal = llvm.builder.CreateLoad(
        Type::getInt32Ty(llvm.context), iVar);

    // A[i], B[i]
    Value* idx[] = {
        ConstantInt::get(Type::getInt32Ty(llvm.context), 0),
        iVal
    };

    Value* aElem =
        llvm.builder.CreateLoad(
            Type::getDoubleTy(llvm.context),
            llvm.builder.CreateGEP(A, idx));

    Value* bElem =
        llvm.builder.CreateLoad(
            Type::getDoubleTy(llvm.context),
            llvm.builder.CreateGEP(B, idx));

    Value* prod = llvm.builder.CreateFMul(aElem, bElem);

    Value* oldSum =
        llvm.builder.CreateLoad(
            Type::getDoubleTy(llvm.context), sum);

    Value* newSum = llvm.builder.CreateFAdd(oldSum, prod);
    llvm.builder.CreateStore(newSum, sum);

    // i++
    Value* nextI = llvm.builder.CreateAdd(
        iVal,
        ConstantInt::get(Type::getInt32Ty(llvm.context), 1));

    llvm.builder.CreateStore(nextI, iVar);

    // i < N ?
    Value* cond =
        llvm.builder.CreateICmpSLT(
            nextI,
            ConstantInt::get(Type::getInt32Ty(llvm.context), N));

    llvm.builder.CreateCondBr(cond, loopBB, afterBB);

    llvm.builder.SetInsertPoint(afterBB);
    return llvm.builder.CreateLoad(
        Type::getDoubleTy(llvm.context), sum);
}
