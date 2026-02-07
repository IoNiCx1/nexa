#pragma once
#include "../ast/Ast.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <map>
#include <string>

struct LLVMState;

class CodeGen {
public:
    explicit CodeGen(LLVMState& state);

    void generate(Program& program);

private:
    LLVMState& llvm;

    std::map<std::string, llvm::Value*> namedValues;

    /* statements */
    void genVarDecl(VarDecl& decl);
    void genArrayDecl(ArrayDecl& decl);
    void genPrintStmt(PrintStmt& stmt, llvm::FunctionCallee& printfFunc);

    /* expressions */
    llvm::Value* genExpression(Expr& expr);

    /* helpers */
    llvm::Type* intType() const;
};
