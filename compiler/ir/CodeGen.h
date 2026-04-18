#ifndef NEXA_CODEGEN_H
#define NEXA_CODEGEN_H

#define LLVM_ENABLE_ABI_BREAKING_CHECKS 0

#include "../ast/Ast.h"
#include "../sema/Type.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>

namespace nexa {

class CodeGen {
public:
    CodeGen();
    void generate(Program& program);
    llvm::Module* getModule();

private:
    // ── Core LLVM objects ─────────────────────
    llvm::LLVMContext              context;
    std::unique_ptr<llvm::Module>  module;
    llvm::IRBuilder<>              builder;

    // ── Variable storage ──────────────────────
    std::unordered_map<std::string, llvm::Value*> namedValues;

    // ── Struct support ────────────────────────
    std::unordered_map<std::string, llvm::StructType*>                   structTypes;
    std::unordered_map<std::string, std::vector<std::pair<std::string,int>>> structFields;

    // ── External runtime functions ────────────
    llvm::Function* printfFunc       = nullptr;
    llvm::Function* mallocFunc       = nullptr;
    llvm::Function* sprintfFunc      = nullptr;
    llvm::Function* aiMatrixFunc     = nullptr;
    llvm::Function* aiMatmulFunc     = nullptr;
    llvm::Function* aiPrintTensorFunc= nullptr;

    // ── File module state ─────────────────────
    bool fileModuleImported = false;

    // ── Helpers ───────────────────────────────
    llvm::Type*  getLLVMType(Type* type);
    void         declareFileRuntime();

    // ── Code generation ───────────────────────
    llvm::Value* generateExpr(Expr* expr);
    llvm::Value* generateFileExpr(FileExpr* fe);
    void         generateStmt(Stmt* stmt);
};

} // namespace nexa

#endif