#pragma once
#include <memory>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

/*
  Central LLVM state for NEXA
*/
struct LLVMState {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    LLVMState()
        : module(std::make_unique<llvm::Module>("nexa", context)),
          builder(context) {}
};
