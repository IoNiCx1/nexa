#pragma once
#include <memory>
#include <string>
#include <vector>

enum class TypeKind { None, Int, Float, Void, Bool };

struct TypeSpec {
    TypeKind kind;
    int rows; 
    int cols;
    TypeSpec() : kind(TypeKind::None), rows(1), cols(1) {}
    TypeSpec(TypeKind k, int r=1, int c=1) : kind(k), rows(r), cols(c) {}
};

struct AstNode {
    virtual ~AstNode() = default;
};

// ==================== EXPRESSIONS ====================

struct Expr : AstNode {
    virtual ~Expr() = default;
};

using ExprPtr = std::unique_ptr<Expr>;

struct IntegerLiteral : Expr {
    int value;
    explicit IntegerLiteral(int v) : value(v) {}
};

// NEW: Float literal
struct FloatLiteral : Expr {
    float value;
    explicit FloatLiteral(float v) : value(v) {}
};

// NEW: String literal
struct StringLiteral : Expr {
    std::string value;
    explicit StringLiteral(const std::string& v) : value(v) {}
};

// NEW: Char literal
struct CharLiteral : Expr {
    char value;
    explicit CharLiteral(char v) : value(v) {}
};

// NEW: Bool literal
struct BoolLiteral : Expr {
    bool value;
    explicit BoolLiteral(bool v) : value(v) {}
};

// NEW: Variable reference (for accessing variables)
struct VarRef : Expr {
    std::string name;
    explicit VarRef(const std::string& n) : name(n) {}
};

// ==================== STATEMENTS ====================

struct Stmt : AstNode {
    virtual ~Stmt() = default;
};

using StmtPtr = std::unique_ptr<Stmt>;

struct VarDecl : Stmt {
    std::string name;
    TypeSpec type;
    ExprPtr initializer;
    VarDecl(std::string n, TypeSpec t, ExprPtr init)
        : name(std::move(n)), type(t), initializer(std::move(init)) {}
};

// NEW: Print statement
struct PrintStmt : Stmt {
    ExprPtr expression;
    explicit PrintStmt(ExprPtr expr) : expression(std::move(expr)) {}
};

// ==================== PROGRAM ====================

struct Program : AstNode {
    std::vector<StmtPtr> statements;
};
