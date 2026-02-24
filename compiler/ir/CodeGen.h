#ifndef NEXA_CODEGEN_H
#define NEXA_CODEGEN_H

#include "../ast/Ast.h"
#include "../sema/Type.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include <unordered_map>
#include <memory>

namespace nexa {

class CodeGen {
public:
    CodeGen();

    void generate(Program& program);
    llvm::Module* getModule();

private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    std::unordered_map<std::string, llvm::Value*> namedValues;

    llvm::Function* printfFunc;
    llvm::Function* mallocFunc;
    llvm::Function* sprintfFunc;

    llvm::Type* getLLVMType(Type* type);

    llvm::Value* generateExpr(Expr* expr);
    void generateStmt(Stmt* stmt);
};

}

#endif