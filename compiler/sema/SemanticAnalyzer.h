#ifndef NEXA_SEMANTIC_ANALYZER_H
#define NEXA_SEMANTIC_ANALYZER_H

#include "../ast/Ast.h"
#include "../sema/Type.h"

#include <unordered_map>
#include <string>

namespace nexa {

class SemanticAnalyzer {
public:
    void analyze(Program& program);

private:
    std::unordered_map<std::string, TypeKind> symbolTable;

    void analyzeStmt(Stmt* stmt);
    TypeKind analyzeExpr(Expr* expr);
};

}

#endif