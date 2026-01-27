#pragma once
#include "../ast/Ast.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <unordered_map>
#include<map>
#include <string>

struct LLVMState; 

class CodeGen {
public:
    CodeGen(LLVMState& state);
    void generate(Program& program);

private:
    LLVMState& llvm;
    std::map<std::string, llvm::Value*> namedValues;

    llvm::Value* genExpression(Expr& expr);
    void genVarDecl(VarDecl& decl);
    void genPrintStmt(PrintStmt& stmt, llvm::FunctionCallee& printfFunc);
    // Add this line:
    void genArrayDecl(ArrayDecl& decl); 
    
    llvm::Type* toLLVMType(const TypeSpec& type);
};
