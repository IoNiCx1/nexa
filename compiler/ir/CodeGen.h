#ifndef NEXA_CODEGEN_H
#define NEXA_CODEGEN_H

#include "../ast/Ast.h"
#include "../sema/Type.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

#include <map>
#include <memory>
#include <string>

namespace nexa {

class CodeGen {
public:
    CodeGen();

    void generate(Program &program);

    llvm::Module* getModule();

private:
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    llvm::Function *printfFunc;

    std::map<std::string, llvm::Value*> namedValues;

    llvm::Type* getLLVMType(const Type &type);

    llvm::Value* generateExpr(Expr *expr);

    void generateStmt(Stmt *stmt);

    llvm::Value* promoteIfNeeded(
        llvm::Value *value,
        const Type &from,
        const Type &to
    );
};

} // namespace nexa

#endif