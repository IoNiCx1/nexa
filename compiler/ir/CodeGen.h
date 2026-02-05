#pragma once
#include "../ast/Ast.h"
#include <llvm/IR/Function.h>
#include <map>
#include <string>

struct LLVMState;

class CodeGen {
public:
    CodeGen(LLVMState& state);
    void generate(Program& program);

private:
    LLVMState& llvm;

    /* variable storage */
    std::map<std::string, llvm::Value*> variables;

    /* variable type tracking (CRITICAL) */
    std::map<std::string, TypeKind> varTypes;

    void genStatement(Stmt& stmt, llvm::FunctionCallee printfFunc);
    llvm::Value* genExpression(Expr& expr);
};
