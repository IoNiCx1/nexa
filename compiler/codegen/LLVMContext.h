#pragma once
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <memory>

struct LLVMState {
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    LLVMState() {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("nexa_module", *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }
};
