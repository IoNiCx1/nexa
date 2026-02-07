#pragma once
#include <memory>
#include <string>
#include <vector>

/* ================= TYPES ================= */

enum class TypeKind {
    None,
    Int,
    Float,
    Bool,
    Char,
    String,

    IntArray,
    CharArray,
    StringArray
};

struct TypeSpec {
    TypeKind kind;
    int rows;
    int cols;

    TypeSpec() : kind(TypeKind::None), rows(1), cols(1) {}
    TypeSpec(TypeKind k, int r = 1, int c = 1)
        : kind(k), rows(r), cols(c) {}
};

/* ================= BASE AST ================= */

struct AstNode {
    virtual ~AstNode() = default;
};

/* ================= EXPRESSIONS ================= */

struct Expr : AstNode {
    virtual ~Expr() = default;
};

using ExprPtr = std::unique_ptr<Expr>;

/* ----- Literals ----- */

struct IntegerLiteral : Expr {
    int value;
    explicit IntegerLiteral(int v) : value(v) {}
};

struct StringLiteral : Expr {
    std::string value;
    explicit StringLiteral(std::string v) : value(std::move(v)) {}
};

struct CharLiteral : Expr {
    char value;
    explicit CharLiteral(char v) : value(v) {}
};

/* ----- Variables ----- */

struct VarRef : Expr {
    std::string name;
    explicit VarRef(std::string n) : name(std::move(n)) {}
};

/* ----- Unary Expression ----- */
/* Example: -x */

enum class UnaryOp {
    Negate
};

struct UnaryExpr : Expr {
    UnaryOp op;
    ExprPtr expr;

    UnaryExpr(UnaryOp o, ExprPtr e)
        : op(o), expr(std::move(e)) {}
};

/* ----- Binary Expression ----- */
/* Example: x + y, a * b */

enum class BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    Mod
};

struct BinaryExpr : Expr {
    BinaryOp op;
    ExprPtr lhs;
    ExprPtr rhs;

    BinaryExpr(BinaryOp o, ExprPtr l, ExprPtr r)
        : op(o), lhs(std::move(l)), rhs(std::move(r)) {}
};

/* ----- Dot Expression (kept for future use) ----- */
/* <a>.<b> */

struct DotExpr : Expr {
    ExprPtr lhs;
    ExprPtr rhs;

    DotExpr(ExprPtr l, ExprPtr r)
        : lhs(std::move(l)), rhs(std::move(r)) {}
};

/* ----- Array Literal (existing) ----- */

struct ArrayLiteral : Expr {
    std::vector<ExprPtr> elements;
    explicit ArrayLiteral(std::vector<ExprPtr> elems)
        : elements(std::move(elems)) {}
};

/* ================= STATEMENTS ================= */

struct Stmt : AstNode {
    virtual ~Stmt() = default;
};

using StmtPtr = std::unique_ptr<Stmt>;

/* ----- Variable Declaration ----- */

struct VarDecl : Stmt {
    std::string name;
    TypeSpec type;
    ExprPtr initializer;

    VarDecl(std::string n, TypeSpec t, ExprPtr init)
        : name(std::move(n)), type(t), initializer(std::move(init)) {}
};

/* ----- Array Declaration ----- */
/* <a> = 1,2,3 */

struct ArrayDecl : Stmt {
    std::string name;
    TypeSpec type;
    std::vector<ExprPtr> elements;

    ArrayDecl(std::string n, TypeSpec t, std::vector<ExprPtr> elems)
        : name(std::move(n)), type(t), elements(std::move(elems)) {}
};

/* ----- Print Statement ----- */

struct PrintStmt : Stmt {
    ExprPtr expr;
    explicit PrintStmt(ExprPtr e) : expr(std::move(e)) {}
};

/* ================= PROGRAM ================= */

struct Program : AstNode {
    std::vector<StmtPtr> statements;
};
