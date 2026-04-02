#ifndef NEXA_CODEGEN_H
#define NEXA_CODEGEN_H

// LLVM 21 compatibility
#define LLVM_ENABLE_ABI_BREAKING_CHECKS 0

#include "../ast/Ast.h"
#include "../sema/Type.h"

#include <memory>
#include <unordered_map>
#include <string>

// LLVM C++ API ONLY (do not include llvm-c headers)
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

    // Core LLVM objects
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;

    // Variable storage
    std::unordered_map<std::string, llvm::Value*> namedValues;

    // External runtime functions
    llvm::Function* printfFunc;
    llvm::Function* mallocFunc;
    llvm::Function* sprintfFunc;
    
    // ai functions 
    llvm::Function* aiMatrixFunc;
    llvm::Function* aiMatmulFunc;
    llvm::Function* aiPrintTensorFunc;

    //Constructor
    llvm::Value* currentSelfPtr = nullptr;
    std::string currentStructName;

    // Type conversion
    llvm::Type* getLLVMType(Type* type);

    // Code generation
    llvm::Value* generateExpr(Expr* expr);
    void generateStmt(Stmt* stmt);

    std::map<std::string, llvm::StructType*> structTypes;
    std::map<std::string, std::vector<std::pair<std::string, int>>> structFields;

};

}

#endif