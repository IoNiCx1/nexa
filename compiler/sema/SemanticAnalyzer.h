#pragma once
#include "../ast/Ast.h"
#include <map>
#include <string>
#include <vector>

namespace nexa {

class SemanticAnalyzer {
public:
    void analyze(Program& program);
    void checkExpr(Expr* expr);
    void checkStmt(Stmt* stmt);

private:
    // Scope stack: each entry maps variable name → Type*
    // push on entering a block, pop on leaving
    std::vector<std::map<std::string, Type*>> symbolStack;
    std::map<std::string, std::vector<std::pair<std::string, Type*>>> structRegistry;

    // ── Scope helpers ─────────────────────────
    void pushScope()  { symbolStack.push_back({}); }
    void popScope()   { symbolStack.pop_back(); }

    // Register a name in the innermost scope
    void declare(const std::string& name, Type* type) {
        symbolStack.back()[name] = type;
    }

    // Look up a name from innermost scope outward; returns nullptr if not found
    Type* lookup(const std::string& name) const {
        for (auto it = symbolStack.rbegin(); it != symbolStack.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) return found->second;
        }
        return nullptr;
    }
};

} // namespace nexa