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
    // Variable Declaration
    // -------------------------
    if (auto var = dynamic_cast<VarDeclStmt*>(stmt)) {

        TypeKind declared =
            var->declaredType->kind;

        TypeKind init =
            analyzeExpr(var->initializer.get());

        if (declared != init) {
            std::cerr << "Type mismatch in variable declaration\n";
            exit(1);
        }

        symbolTable[var->name] = declared;
    }

    // -------------------------
    // Assignment
    // -------------------------
    else if (auto assign =
        dynamic_cast<AssignmentStmt*>(stmt)) {

        auto varExpr =
            dynamic_cast<VariableExpr*>(assign->target.get());

        if (!varExpr) {
            std::cerr << "Invalid assignment target\n";
            exit(1);
        }

        if (!symbolTable.count(varExpr->name)) {
            std::cerr << "Undefined variable: "
                      << varExpr->name << "\n";
            exit(1);
        }

        TypeKind targetType =
            symbolTable[varExpr->name];

        TypeKind valueType =
            analyzeExpr(assign->value.get());

        if (targetType != valueType) {
            std::cerr << "Assignment type mismatch\n";
            exit(1);
        }
    }

    // -------------------------
    // Print
    // -------------------------
    else if (auto print =
        dynamic_cast<PrintStmt*>(stmt)) {

        analyzeExpr(print->expression.get());
    }

    // -------------------------
    // Loop
    // -------------------------
    else if (auto loop =
        dynamic_cast<LoopStmt*>(stmt)) {

        TypeKind countType =
            analyzeExpr(loop->count.get());

        if (countType != TypeKind::Int) {
            std::cerr << "Loop count must be int\n";
            exit(1);
        }

        symbolTable[loop->iterator] = TypeKind::Int;

        for (auto& bodyStmt : loop->body)
            analyzeStmt(bodyStmt.get());
    }
}

// =============================
// Expressions
// =============================

TypeKind SemanticAnalyzer::analyzeExpr(Expr* expr) {

    // -------------------------
    // Integer
    // -------------------------
    if (auto intLit =
        dynamic_cast<IntegerLiteral*>(expr)) {

        expr->inferredType =
            new Type(TypeKind::Int);

        return TypeKind::Int;
    }

    // -------------------------
    // Double
    // -------------------------
    if (auto dblLit =
        dynamic_cast<DoubleLiteral*>(expr)) {

        expr->inferredType =
            new Type(TypeKind::Double);

        return TypeKind::Double;
    }

    // -------------------------
    // String
    // -------------------------
    if (auto strLit =
        dynamic_cast<StringLiteral*>(expr)) {

        expr->inferredType =
            new Type(TypeKind::String);

        return TypeKind::String;
    }

    // -------------------------
    // Boolean
    // -------------------------
    if (auto boolLit = 
    dynamic_cast<BooleanLiteral*>(expr))
    {
        expr->inferredType = 
            new Type(TypeKind::Bool);

        return TypeKind::Bool;
    }

    // -------------------------
    // Array Literal
    // -------------------------
    if (auto arr =
        dynamic_cast<ArrayLiteralExpr*>(expr)) {

        if (arr->elements.empty()) {
            std::cerr << "Empty array not allowed\n";
            exit(1);
        }

        TypeKind first =
            analyzeExpr(arr->elements[0].get());

        for (auto& el : arr->elements) {
            TypeKind t =
                analyzeExpr(el.get());

            if (t != first) {
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
    if (auto index =
        dynamic_cast<IndexExpr*>(expr)) {

        TypeKind arrayType =
            analyzeExpr(index->array.get());

        if (arrayType != TypeKind::Array) {
            std::cerr << "Cannot index non-array type\n";
            exit(1);
        }

        TypeKind idxType =
            analyzeExpr(index->index.get());

        if (idxType != TypeKind::Int) {
            std::cerr << "Array index must be int\n";
            exit(1);
        }

        // Assume int element type for now
        expr->inferredType = new Type(TypeKind::Int);
    return TypeKind::Int;
    }

    // -------------------------
    // Variable
    // -------------------------
    if (auto var =
        dynamic_cast<VariableExpr*>(expr)) {

        if (!symbolTable.count(var->name)) {
            std::cerr << "Undefined variable: "
                      << var->name << "\n";
            exit(1);
        }

        TypeKind t =
            symbolTable[var->name];

        expr->inferredType =
            new Type(t);

        return t;
    }

    // -------------------------
    // Binary
    // -------------------------
    if (auto bin =
        dynamic_cast<BinaryExpr*>(expr)) {

        TypeKind left =
            analyzeExpr(bin->left.get());

        TypeKind right =
            analyzeExpr(bin->right.get());

        // Block string arithmetic
        if (left == TypeKind::String ||
            right == TypeKind::String)
        {
            std::cerr << "String operations not supported\n";
            exit(1);
        }

        // Same type → OK
        if (left == right)
        {
            expr->inferredType =
                new Type(left);
            return left;
        }

        // int + double OR double + int
        if ((left == TypeKind::Int &&
            right == TypeKind::Double) ||
            (left == TypeKind::Double &&
            right == TypeKind::Int))
        {
            expr->inferredType =
                new Type(TypeKind::Double);
            return TypeKind::Double;
        }

        std::cerr << "Binary type mismatch\n";
        exit(1);
    }

    std::cerr << "Unknown expression\n";
    exit(1);
}