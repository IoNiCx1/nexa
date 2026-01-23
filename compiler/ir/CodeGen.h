#pragma once

#include "../ast/Ast.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <memory>

/*
  Shared LLVM state
*/
struct LLVMState {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    LLVMState()
        : module(std::make_unique<llvm::Module>("nexa", context)),
          builder(context) {}
};

/*
  Code generation entry
*/
class CodeGen {
public:
    explicit CodeGen(LLVMState& state);

    void generate(Program& program);

private:
    LLVMState& llvm;
};
