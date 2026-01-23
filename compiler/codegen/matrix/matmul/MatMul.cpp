#include "MatMul.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

MatMul::MatMul(LLVMState& state) : llvm(state) {}

llvm::Value* MatMul::generate(
    llvm::Value* A,
    llvm::Value* B,
    int M,
    int K,
    int N
) {
    using namespace llvm;

    Function* func = llvm.builder.GetInsertBlock()->getParent();

    // Allocate C[M*N]
    ArrayType* cType =
        ArrayType::get(Type::getDoubleTy(llvm.context), M * N);

    Value* C =
        llvm.builder.CreateAlloca(cType, nullptr, "matmul.C");

    // i loop
    AllocaInst* iVar =
        llvm.builder.CreateAlloca(Type::getInt32Ty(llvm.context), nullptr, "i");
    llvm.builder.CreateStore(ConstantInt::get(Type::getInt32Ty(llvm.context), 0), iVar);

    BasicBlock* iLoop = BasicBlock::Create(llvm.context, "i.loop", func);
    BasicBlock* iAfter = BasicBlock::Create(llvm.context, "i.after", func);
    llvm.builder.CreateBr(iLoop);
    llvm.builder.SetInsertPoint(iLoop);

    Value* iVal = llvm.builder.CreateLoad(Type::getInt32Ty(llvm.context), iVar);

    // j loop
    AllocaInst* jVar =
        llvm.builder.CreateAlloca(Type::getInt32Ty(llvm.context), nullptr, "j");
    llvm.builder.CreateStore(ConstantInt::get(Type::getInt32Ty(llvm.context), 0), jVar);

    BasicBlock* jLoop = BasicBlock::Create(llvm.context, "j.loop", func);
    BasicBlock* jAfter = BasicBlock::Create(llvm.context, "j.after", func);
    llvm.builder.CreateBr(jLoop);
    llvm.builder.SetInsertPoint(jLoop);

    Value* jVal = llvm.builder.CreateLoad(Type::getInt32Ty(llvm.context), jVar);

    // sum = 0
    AllocaInst* sum =
        llvm.builder.CreateAlloca(Type::getDoubleTy(llvm.context), nullptr, "sum");
    llvm.builder.CreateStore(ConstantFP::get(Type::getDoubleTy(llvm.context), 0.0), sum);

    // k loop
    AllocaInst* kVar =
        llvm.builder.CreateAlloca(Type::getInt32Ty(llvm.context), nullptr, "k");
    llvm.builder.CreateStore(ConstantInt::get(Type::getInt32Ty(llvm.context), 0), kVar);

    BasicBlock* kLoop = BasicBlock::Create(llvm.context, "k.loop", func);
    BasicBlock* kAfter = BasicBlock::Create(llvm.context, "k.after", func);
    llvm.builder.CreateBr(kLoop);
    llvm.builder.SetInsertPoint(kLoop);

    Value* kVal = llvm.builder.CreateLoad(Type::getInt32Ty(llvm.context), kVar);

    // A[i*K + k]
    Value* aIndex = llvm.builder.CreateAdd(
        llvm.builder.CreateMul(iVal, ConstantInt::get(Type::getInt32Ty(llvm.context), K)),
        kVal);

    Value* aPtr = llvm.builder.CreateGEP(
        A,
        {
            ConstantInt::get(Type::getInt32Ty(llvm.context), 0),
            aIndex
        });

    Value* aElem =
        llvm.builder.CreateLoad(Type::getDoubleTy(llvm.context), aPtr);

    // B[k*N + j]
    Value* bIndex = llvm.builder.CreateAdd(
        llvm.builder.CreateMul(kVal, ConstantInt::get(Type::getInt32Ty(llvm.context), N)),
        jVal);

    Value* bPtr = llvm.builder.CreateGEP(
        B,
        {
            ConstantInt::get(Type::getInt32Ty(llvm.context), 0),
            bIndex
        });

    Value* bElem =
        llvm.builder.CreateLoad(Type::getDoubleTy(llvm.context), bPtr);

    // sum += A * B
    Value* prod = llvm.builder.CreateFMul(aElem, bElem);
    Value* oldSum = llvm.builder.CreateLoad(Type::getDoubleTy(llvm.context), sum);
    llvm.builder.CreateStore(
        llvm.builder.CreateFAdd(oldSum, prod), sum);

    // k++
    llvm.builder.CreateStore(
        llvm.builder.CreateAdd(
            kVal,
            ConstantInt::get(Type::getInt32Ty(llvm.context), 1)),
        kVar);

    Value* kCond =
        llvm.builder.CreateICmpSLT(
            llvm.builder.CreateLoad(Type::getInt32Ty(llvm.context), kVar),
            ConstantInt::get(Type::getInt32Ty(llvm.context), K));

    llvm.builder.CreateCondBr(kCond, kLoop, kAfter);
    llvm.builder.SetInsertPoint(kAfter);

    // C[i*N + j] = sum
    Value* cIndex = llvm.builder.CreateAdd(
        llvm.builder.CreateMul(iVal, ConstantInt::get(Type::getInt32Ty(llvm.context), N)),
        jVal);

    Value* cPtr = llvm.builder.CreateGEP(
        C,
        {
            ConstantInt::get(Type::getInt32Ty(llvm.context), 0),
            cIndex
        });

    llvm.builder.CreateStore(
        llvm.builder.CreateLoad(Type::getDoubleTy(llvm.context), sum),
        cPtr);

    // j++
    llvm.builder.CreateStore(
        llvm.builder.CreateAdd(
            jVal,
            ConstantInt::get(Type::getInt32Ty(llvm.context), 1)),
        jVar);

    Value* jCond =
        llvm.builder.CreateICmpSLT(
            llvm.builder.CreateLoad(Type::getInt32Ty(llvm.context), jVar),
            ConstantInt::get(Type::getInt32Ty(llvm.context), N));

    llvm.builder.CreateCondBr(jCond, jLoop, jAfter);
    llvm.builder.SetInsertPoint(jAfter);

    // i++
    llvm.builder.CreateStore(
        llvm.builder.CreateAdd(
            iVal,
            ConstantInt::get(Type::getInt32Ty(llvm.context), 1)),
        iVar);

    Value* iCond =
        llvm.builder.CreateICmpSLT(
            llvm.builder.CreateLoad(Type::getInt32Ty(llvm.context), iVar),
            ConstantInt::get(Type::getInt32Ty(llvm.context), M));

    llvm.builder.CreateCondBr(iCond, iLoop, iAfter);
    llvm.builder.SetInsertPoint(iAfter);

    return C;
}
