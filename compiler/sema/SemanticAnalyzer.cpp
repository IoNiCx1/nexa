#include "SemanticAnalyzer.h"

/* ================= PROGRAM ================= */

void SemanticAnalyzer::analyze(Program& program) {
    for (auto& stmt : program.statements) {
        analyzeStmt(stmt.get());
    }
}

/* ================= STATEMENTS ================= */

void SemanticAnalyzer::analyzeStmt(Stmt* stmt) {

    if (auto* v = dynamic_cast<VarDecl*>(stmt)) {
        if (v->initializer)
            analyzeExpr(v->initializer.get());
        return;
    }

    if (auto* p = dynamic_cast<PrintStmt*>(stmt)) {
        analyzeExpr(p->expr.get());
        return;
    }

    if (auto* a = dynamic_cast<ArrayDecl*>(stmt)) {
        for (auto& e : a->elements)
            analyzeExpr(e.get());
        return;
    }
}

/* ================= EXPRESSIONS ================= */

void SemanticAnalyzer::analyzeExpr(Expr* expr) {

    if (dynamic_cast<IntegerLiteral*>(expr)) return;
    if (dynamic_cast<StringLiteral*>(expr)) return;
    if (dynamic_cast<CharLiteral*>(expr)) return;
    if (dynamic_cast<VarRef*>(expr)) return;

    if (auto* u = dynamic_cast<UnaryExpr*>(expr)) {
        analyzeExpr(u->expr.get());
        return;
    }

    if (auto* b = dynamic_cast<BinaryExpr*>(expr)) {
        analyzeExpr(b->lhs.get());
        analyzeExpr(b->rhs.get());
        return;
    }

    if (auto* a = dynamic_cast<ArrayLiteral*>(expr)) {
        for (auto& e : a->elements)
            analyzeExpr(e.get());
        return;
    }
}
