#pragma once
#include <memory>
#include <string>
#include <vector>

/* ================= TYPES ================= */

enum class TypeKind {
    None,
    Int,
    String,
    IntArray,
    StringArray
};

struct TypeSpec {
    TypeKind kind;
    TypeSpec(TypeKind k = TypeKind::None) : kind(k) {}
};

/* ================= BASE ================= */

struct AstNode {
    virtual ~AstNode() = default;
};

/* ================= EXPRESSIONS ================= */

struct Expr : AstNode {};
using ExprPtr = std::unique_ptr<Expr>;

struct IntegerLiteral : Expr {
    int value;
    explicit IntegerLiteral(int v) : value(v) {}
};

struct StringLiteral : Expr {
    std::string value;
    explicit StringLiteral(std::string v) : value(std::move(v)) {}
};

struct VarRef : Expr {
    std::string name;
    explicit VarRef(std::string n) : name(std::move(n)) {}
};

/* NEW: typed reference expression: <a>, {s} */
struct TypedRefExpr : Expr {
    std::string name;
    TypeKind type;
    TypedRefExpr(std::string n, TypeKind t)
        : name(std::move(n)), type(t) {}
};

struct CallExpr : Expr {
    std::string functionName;
    explicit CallExpr(std::string n) : functionName(std::move(n)) {}
};

/* ================= STATEMENTS ================= */

struct Stmt : AstNode {};
using StmtPtr = std::unique_ptr<Stmt>;

struct PrintStmt : Stmt {
    ExprPtr expr;
    explicit PrintStmt(ExprPtr e) : expr(std::move(e)) {}
};

struct ExprStmt : Stmt {
    ExprPtr expr;
    explicit ExprStmt(ExprPtr e) : expr(std::move(e)) {}
};

struct VarDecl : Stmt {
    std::string name;
    TypeSpec type;
    ExprPtr init;
    VarDecl(std::string n, TypeSpec t, ExprPtr i)
        : name(std::move(n)), type(t), init(std::move(i)) {}
};

struct ArrayDecl : Stmt {
    std::string name;
    TypeSpec type;
    std::vector<ExprPtr> elements;
    ArrayDecl(std::string n, TypeSpec t, std::vector<ExprPtr> e)
        : name(std::move(n)), type(t), elements(std::move(e)) {}
};

/* ================= PROGRAM ================= */

struct Program : AstNode {
    std::vector<StmtPtr> statements;
};
