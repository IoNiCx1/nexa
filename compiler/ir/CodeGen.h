#pragma once
#include "../ast/Ast.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <unordered_map>
#include <map>
#include <string>

struct LLVMState;

class CodeGen {
public:
    CodeGen(LLVMState& state);
    void generate(Program& program);

private:
    LLVMState& llvm;
    std::map<std::string, llvm::Value*> namedValues;
    std::map<std::string, TypeSpec> namedTypes;

    llvm::Value* genExpression(Expr& expr);

    void genVarDecl(VarDecl& decl);
    void genArrayDecl(ArrayDecl& decl);
    void genPrintStmt(PrintStmt& stmt, llvm::FunctionCallee& printfFunc);

    void printArray(const std::string& name,
                    llvm::Value* arr,
                    const TypeSpec& type,
                    llvm::FunctionCallee& printfFunc);

    llvm::Type* toLLVMType(const TypeSpec& type);
};

