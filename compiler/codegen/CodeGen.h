#pragma once
#include "LLVMContext.h"
#include "../ast/Ast.h"
#include <unordered_map>
#include <string>

class CodeGen {
public:
    explicit CodeGen(LLVMState& state);

    void generate(Program& program);

private:
    LLVMState& llvm;
    std::unordered_map<std::string, llvm::Value*> namedValues;

    void genDeclaration(Declaration& decl);
    llvm::Value* genExpression(Expr& expr);

    llvm::Type* toLLVMType(const TypeSpec& type);
};
