#ifndef NEXA_AST_H
#define NEXA_AST_H

#include <memory>
#include <string>
#include <vector>

namespace nexa {

struct Type;

// =============================
// Base Nodes
// =============================

struct Expr {
    virtual ~Expr() = default;
    Type* inferredType = nullptr;
};

struct Stmt {
    virtual ~Stmt() = default;
};

// =============================
// Program
// =============================

struct Program {
    std::vector<std::unique_ptr<Stmt>> statements;
};

// =============================
// Literals
// =============================

struct IntegerLiteral : Expr {
    int value;
    IntegerLiteral(int v) : value(v) {}
};

struct DoubleLiteral : Expr {
    double value;
    DoubleLiteral(double v) : value(v) {}
};

struct StringLiteral : Expr {
    std::string value;
    StringLiteral(const std::string& v) : value(v) {}
};

// =============================
// Variable
// =============================

struct VariableExpr : Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
};

// =============================
// Array Literal
// =============================

struct ArrayLiteralExpr : Expr {
    std::vector<std::unique_ptr<Expr>> elements;
};

// =============================
// Index Expression
// =============================

struct IndexExpr : Expr {
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;

    IndexExpr(std::unique_ptr<Expr> arr,
              std::unique_ptr<Expr> idx)
        : array(std::move(arr)),
          index(std::move(idx)) {}
};

// =============================
// Unary / Binary
// =============================

struct UnaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> operand;

    UnaryExpr(const std::string& o,
              std::unique_ptr<Expr> expr)
        : op(o), operand(std::move(expr)) {}
};

struct BinaryExpr : Expr {
    std::string op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l,
               const std::string& o,
               std::unique_ptr<Expr> r)
        : op(o),
          left(std::move(l)),
          right(std::move(r)) {}
};

// =============================
// Function Call
// =============================

struct CallExpr : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> arguments;
};

// =============================
// Statements
// =============================

struct VarDeclStmt : Stmt {
    Type* declaredType;
    std::string name;
    std::unique_ptr<Expr> initializer;

    VarDeclStmt(Type* t,
                const std::string& n,
                std::unique_ptr<Expr> init)
        : declaredType(t),
          name(n),
          initializer(std::move(init)) {}
};

struct AssignmentStmt : Stmt {
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;

    AssignmentStmt(std::unique_ptr<Expr> t,
                   std::unique_ptr<Expr> v)
        : target(std::move(t)),
          value(std::move(v)) {}
};

struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expression;

    PrintStmt(std::unique_ptr<Expr> e)
        : expression(std::move(e)) {}
};

// =============================
// Loop
// =============================

struct LoopStmt : Stmt {
    std::string iterator;
    std::unique_ptr<Expr> count;
    std::vector<std::unique_ptr<Stmt>> body;

    LoopStmt(const std::string& it,
             std::unique_ptr<Expr> c)
        : iterator(it),
          count(std::move(c)) {}
};

} // namespace nexa

#endif