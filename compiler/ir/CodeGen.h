#pragma once
#include "../ast/Ast.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <unordered_map>
#include <string>

struct LLVMState; 

class CodeGen {
public:
    explicit CodeGen(LLVMState& state);
    void generate(Program& program);

private:
    LLVMState& llvm;
    std::unordered_map<std::string, llvm::Value*> namedValues;
    
    void genVarDecl(VarDecl& decl);
    
    // NEW: Generate print statement
    void genPrintStmt(PrintStmt& stmt, llvm::FunctionCallee& printfFunc);
    
    llvm::Value* genExpression(Expr& expr);
    llvm::Type* toLLVMType(const TypeSpec& type);
};
