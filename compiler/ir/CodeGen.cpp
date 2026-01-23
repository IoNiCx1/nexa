#include "CodeGen.h"
#include <llvm/IR/Verifier.h>

CodeGen::CodeGen(LLVMState& state)
    : llvm(state) {}

void CodeGen::generate(Program&) {
    using namespace llvm;

    // int main()
    FunctionType* mainType =
        FunctionType::get(Type::getInt32Ty(llvm.context), false);

    Function* mainFn =
        Function::Create(
            mainType,
            Function::ExternalLinkage,
            "main",
            llvm.module.get());

    BasicBlock* entry =
        BasicBlock::Create(llvm.context, "entry", mainFn);

    llvm.builder.SetInsertPoint(entry);

    llvm.builder.CreateRet(
        ConstantInt::get(Type::getInt32Ty(llvm.context), 0));

    verifyFunction(*mainFn);
}
