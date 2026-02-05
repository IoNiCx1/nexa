#include "SemanticAnalyzer.h"
#include "../ast/Ast.h"
#include <stdexcept>

void SemanticAnalyzer::analyze(Program& program) {
    for (auto& stmt : program.statements) {
        analyzeStmt(stmt.get());
    }
}

void SemanticAnalyzer::analyzeStmt(Stmt* stmt) {

    /* print statement */
    if (auto* p = dynamic_cast<PrintStmt*>(stmt)) {
        analyzeExpr(p->expr.get());
        return;
    }

    /* variable declaration */
    if (auto* v = dynamic_cast<VarDecl*>(stmt)) {
        if (v->init)
            analyzeExpr(v->init.get());
        return;
    }

    /* array declaration */
    if (auto* a = dynamic_cast<ArrayDecl*>(stmt)) {
        for (auto& e : a->elements)
            analyzeExpr(e.get());
        return;
    }

    throw std::runtime_error("Unsupported statement in semantic analysis");
}

void SemanticAnalyzer::analyzeExpr(Expr* expr) {

    if (dynamic_cast<IntegerLiteral*>(expr)) return;
    if (dynamic_cast<StringLiteral*>(expr)) return;   // ✅ allows: print "hello"
    if (dynamic_cast<VarRef*>(expr)) return;
    if (dynamic_cast<TypedRefExpr*>(expr)) return;

    throw std::runtime_error("Invalid expression");
}
