#pragma once

#include "SymbolTable.h"
#include "../ast/Ast.h"

class SemanticAnalyzer {
public:
    SemanticAnalyzer() = default;

    // Entry point: walks the AST
    void analyze(Program& program);

private:
    SymbolTable symbols;

    // Helper to map AST types to Sema types
    SemanticType lowerType(const TypeSpec& t);

    // Analysis for specific nodes
    void analyzeVarDecl(VarDecl& decl);
};
