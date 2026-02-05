#pragma once
#include "../ast/Ast.h"

class SemanticAnalyzer {
public:
    void analyze(Program& program);

private:
    void analyzeStmt(Stmt* stmt);
    void analyzeExpr(Expr* expr);
};
