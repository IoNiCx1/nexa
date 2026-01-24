#pragma once
#include <memory>
#include <string>
#include <vector>

enum class TypeKind { None, Int, Float, Void, Bool };

struct TypeSpec {
    TypeKind kind;
    int rows; 
    int cols;

    TypeSpec() : kind(TypeKind::None), rows(0), cols(0) {}
    TypeSpec(TypeKind k, int r, int c) : kind(k), rows(r), cols(c) {}
};

struct AstNode {
    virtual ~AstNode() = default;
};

struct Expr : AstNode {
    virtual ~Expr() = default;
};
using ExprPtr = std::unique_ptr<Expr>;

struct IntegerLiteral : Expr {
    int value;
    explicit IntegerLiteral(int v) : value(v) {}
};

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

struct Program : AstNode {
    std::vector<StmtPtr> statements;
};
