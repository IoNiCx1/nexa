#include "SemanticAnalyzer.h"
#include <iostream>
#include <stdexcept>

using namespace nexa;

void SemanticAnalyzer::analyze(Program& program) {
    // Create global scope
    symbolStack.push_back({});

    for (auto& stmt : program.statements) {
        checkStmt(stmt.get());
    }
}

void SemanticAnalyzer::checkStmt(Stmt* stmt) {
    if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
        if (varDecl->initializer) {
            checkExpr(varDecl->initializer.get());
        }
        // Register variable in current scope
        symbolStack.back()[varDecl->name] = varDecl->declaredType;
    } 
    else if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
        checkExpr(print->expression.get());
    }
    else if (auto assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        checkExpr(assign->value.get());
        if (auto varExpr = dynamic_cast<VariableExpr*>(assign->target.get())) {
            // In a real compiler, you'd check if variable exists here
        }
    }
    else if (auto loop = dynamic_cast<LoopStmt*>(stmt)) {
        checkExpr(loop->count.get());
        
        // Add scope for loop
        symbolStack.push_back({});
        symbolStack.back()[loop->iterator] = &TYPE_INT;
        
        for (auto& s : loop->body) {
            checkStmt(s.get());
        }
        
        symbolStack.pop_back();
    }
}

void SemanticAnalyzer::checkExpr(Expr* expr) {
    if (!expr) return;

    // Literals
    if (dynamic_cast<IntegerLiteral*>(expr)) {
        expr->inferredType = &TYPE_INT;
    } 
    else if (dynamic_cast<DoubleLiteral*>(expr)) {
        expr->inferredType = &TYPE_DOUBLE;
    }
    else if (dynamic_cast<StringLiteral*>(expr)) {
        expr->inferredType = &TYPE_STRING;
    }
    else if (dynamic_cast<BoolLiteral*>(expr)) {
        expr->inferredType = &TYPE_BOOL;
    }
    
    // Grouping (Parentheses)
    else if (auto group = dynamic_cast<GroupingExpr*>(expr)) {
        checkExpr(group->expression.get());
        expr->inferredType = group->expression->inferredType;
    }

    // Tensors
    else if (auto tensorLit = dynamic_cast<TensorLiteralExpr*>(expr)) {
        expr->inferredType = &TYPE_TENSOR;
        for (auto& row : tensorLit->rows) {
            for (auto& element : row) {
                checkExpr(element.get());
            }
        }
    }

    // Variables
    else if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        // Look up in symbol stack (from top to bottom)
        bool found = false;
        for (auto it = symbolStack.rbegin(); it != symbolStack.rend(); ++it) {
            if (it->count(var->name)) {
                expr->inferredType = (*it)[var->name];
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "Semantic Error: Undefined variable '" << var->name << "'\n";
        }
    }

    // Binary Operations
    else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        checkExpr(bin->left.get());
        checkExpr(bin->right.get());

        // Basic type promotion: if either side is double, result is double
        if (bin->left->inferredType->isDouble() || bin->right->inferredType->isDouble()) {
            expr->inferredType = &TYPE_DOUBLE;
        } else if (bin->left->inferredType->isTensor()) {
            expr->inferredType = &TYPE_TENSOR;
        } else {
            expr->inferredType = &TYPE_INT;
        }
    }
}