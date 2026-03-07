#include "SemanticAnalyzer.h"
#include <iostream>
#include <cstdlib>

using namespace nexa;

// =============================
// Entry
// =============================

void SemanticAnalyzer::analyze(Program& program) {
    for (auto& stmt : program.statements)
        analyzeStmt(stmt.get());
}

// =============================
// Statements
// =============================

void SemanticAnalyzer::analyzeStmt(Stmt* stmt) {

    // -------------------------
    // Function Declaration
    // -------------------------
    if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {
        
        // Register the function name in the global symbol table 
        // so it can be called later (and support recursion).
        symbolTable[fn->name] = fn->returnType->kind;

        // Save old symbol table to create a new scope for the function body
        auto oldTable = symbolTable;

        // Register parameters in the local scope
        for (auto& p : fn->params) {
            symbolTable[p.first] = p.second->kind;
        }

        // Analyze function body
        for (auto& s : fn->body)
            analyzeStmt(s.get());

        // Restore scope (parameters and local vars shouldn't exist outside)
        symbolTable = oldTable;
    }

    // -------------------------
    // Return Statement
    // -------------------------
    else if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
        analyzeExpr(ret->value.get());
    }

    // -------------------------
    // Variable Declaration
    // -------------------------
    else if (auto var = dynamic_cast<VarDeclStmt*>(stmt)) {
        TypeKind declared = var->declaredType->kind;
        TypeKind init = analyzeExpr(var->initializer.get());

        if (declared != init) {
            std::cerr << "Type mismatch in variable declaration: " << var->name << "\n";
            exit(1);
        }

        symbolTable[var->name] = declared;
    }

    // -------------------------
    // Assignment
    // -------------------------
    else if (auto assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        auto varExpr = dynamic_cast<VariableExpr*>(assign->target.get());

        if (!varExpr) {
            std::cerr << "Invalid assignment target\n";
            exit(1);
        }

        if (!symbolTable.count(varExpr->name)) {
            std::cerr << "Undefined variable: " << varExpr->name << "\n";
            exit(1);
        }

        TypeKind targetType = symbolTable[varExpr->name];
        TypeKind valueType = analyzeExpr(assign->value.get());

        if (targetType != valueType) {
            std::cerr << "Assignment type mismatch for " << varExpr->name << "\n";
            exit(1);
        }
    }

    // -------------------------
    // Print
    // -------------------------
    else if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
        analyzeExpr(print->expression.get());
    }

    // -------------------------
    // Loop
    // -------------------------
    else if (auto loop = dynamic_cast<LoopStmt*>(stmt)) {
        TypeKind countType = analyzeExpr(loop->count.get());

        if (countType != TypeKind::Int) {
            std::cerr << "Loop count must be int\n";
            exit(1);
        }

        // Iterator variable scope
        auto oldTable = symbolTable;
        symbolTable[loop->iterator] = TypeKind::Int;

        for (auto& bodyStmt : loop->body)
            analyzeStmt(bodyStmt.get());
            
        symbolTable = oldTable;
    }

    // -------------------------
    // If Statement
    // -------------------------
    else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        TypeKind condType = analyzeExpr(ifStmt->condition.get());

        if (condType != TypeKind::Bool) {
            std::cerr << "If condition must be bool\n";
            exit(1);
        }

        for (auto& s : ifStmt->thenBranch)
            analyzeStmt(s.get());

        for (auto& s : ifStmt->elseBranch)
            analyzeStmt(s.get());
    }
}

// =============================
// Expressions
// =============================

TypeKind SemanticAnalyzer::analyzeExpr(Expr* expr) {

    // -------------------------
    // Integer
    // -------------------------
    if (auto intLit = dynamic_cast<IntegerLiteral*>(expr)) {
        expr->inferredType = new Type(TypeKind::Int);
        return TypeKind::Int;
    }

    // -------------------------
    // Double
    // -------------------------
    if (auto dblLit = dynamic_cast<DoubleLiteral*>(expr)) {
        expr->inferredType = new Type(TypeKind::Double);
        return TypeKind::Double;
    }

    // -------------------------
    // String
    // -------------------------
    if (auto strLit = dynamic_cast<StringLiteral*>(expr)) {
        expr->inferredType = new Type(TypeKind::String);
        return TypeKind::String;
    }

    // -------------------------
    // Bool
    // -------------------------
    if (auto boolLit = dynamic_cast<BoolLiteral*>(expr)) {
        expr->inferredType = new Type(TypeKind::Bool);
        return TypeKind::Bool;
    }

    // -------------------------
    // Array Literal
    // -------------------------
    if (auto arr = dynamic_cast<ArrayLiteralExpr*>(expr)) {
        if (arr->elements.empty()) {
            std::cerr << "Empty array not allowed\n";
            exit(1);
        }

        TypeKind first = analyzeExpr(arr->elements[0].get());
        for (auto& el : arr->elements) {
            if (analyzeExpr(el.get()) != first) {
                std::cerr << "Array elements must have same type\n";
                exit(1);
            }
        }

        expr->inferredType = new Type(TypeKind::Array);
        return TypeKind::Array;
    }

    // -------------------------
    // Indexing
    // -------------------------
    if (auto index = dynamic_cast<IndexExpr*>(expr)) {
        if (analyzeExpr(index->array.get()) != TypeKind::Array) {
            std::cerr << "Cannot index non-array type\n";
            exit(1);
        }
        if (analyzeExpr(index->index.get()) != TypeKind::Int) {
            std::cerr << "Array index must be int\n";
            exit(1);
        }
        expr->inferredType = new Type(TypeKind::Int);
        return TypeKind::Int;
    }

    // -------------------------
    // Variable
    // -------------------------
    if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        if (!symbolTable.count(var->name)) {
            std::cerr << "Undefined identifier: " << var->name << "\n";
            exit(1);
        }
        TypeKind t = symbolTable[var->name];
        expr->inferredType = new Type(t);
        return t;
    }

    // -------------------------
    // Call Expression (The Missing Link)
    // -------------------------
    if (auto call = dynamic_cast<CallExpr*>(expr)) {
        if (!symbolTable.count(call->callee)) {
            std::cerr << "Undefined function: " << call->callee << "\n";
            exit(1);
        }

        // Analyze arguments to ensure they are valid expressions
        for (auto& arg : call->arguments) {
            analyzeExpr(arg.get());
        }

        // The type of a call expression is the function's return type
        TypeKind returnType = symbolTable[call->callee];
        expr->inferredType = new Type(returnType);
        return returnType;
    }

    // -------------------------
    // Binary
    // -------------------------
    if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        TypeKind left = analyzeExpr(bin->left.get());
        TypeKind right = analyzeExpr(bin->right.get());

        if (left != right) {
            std::cerr << "Binary type mismatch\n";
            exit(1);
        }

        std::string op = bin->op;
        if (op == "<"  || op == ">"  || op == "<=" || op == ">=" ||
            op == "==" || op == "!=") {
            expr->inferredType = new Type(TypeKind::Bool);
            return TypeKind::Bool;
        }

        expr->inferredType = new Type(left);
        return left;
    }

    std::cerr << "Unknown expression: analysis failed\n";
    exit(1);
}