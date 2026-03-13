#pragma once

#include "../ast/Ast.h"
#include <map>
#include <string>
#include <vector>

namespace nexa {

class SemanticAnalyzer {
public:
    void analyze(Program& program);
    
    // This declaration was missing!
    void checkExpr(Expr* expr);
    void checkStmt(Stmt* stmt);

private:
    // Scope stack for variables: maps name to Type pointer
    std::vector<std::map<std::string, Type*>> symbolStack;
};

} // namespace nexa