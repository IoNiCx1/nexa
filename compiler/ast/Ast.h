#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../sema/Type.h" // Use the single source of truth

namespace nexa {

// =============================
// Expressions
// =============================

struct Expr {
    virtual ~Expr() = default;
    Type* inferredType = nullptr;
};

struct IntegerLiteral : public Expr {
    int value;
    IntegerLiteral(int val) : value(val) {}
};

struct DoubleLiteral : public Expr {
    double value;
    DoubleLiteral(double val) : value(val) {}
};

struct StringLiteral : public Expr {
    std::string value;
    StringLiteral(const std::string& val) : value(val) {}
};

struct BoolLiteral : public Expr {
    bool value;
    BoolLiteral(bool val) : value(val) {}
};

struct GroupingExpr : public Expr {
    std::unique_ptr<Expr> expression;
    GroupingExpr(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
};

struct VariableExpr : public Expr {
    std::string name;
    VariableExpr(const std::string& n) : name(n) {}
};

struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> L, std::string O, std::unique_ptr<Expr> R)
        : left(std::move(L)), op(O), right(std::move(R)) {}
};

struct ArrayLiteralExpr : public Expr {
    std::vector<std::unique_ptr<Expr>> elements;
};

struct IndexExpr : public Expr {
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;
    IndexExpr(std::unique_ptr<Expr> arr, std::unique_ptr<Expr> idx)
        : array(std::move(arr)), index(std::move(idx)) {}
};

struct TensorLiteralExpr : public Expr {
    std::vector<std::vector<std::unique_ptr<Expr>>> rows;
    TensorLiteralExpr(std::vector<std::vector<std::unique_ptr<Expr>>> r)
        : rows(std::move(r)) {}
};

struct CallExpr : public Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(const std::string& c, std::vector<std::unique_ptr<Expr>> args)
        : callee(c), arguments(std::move(args)) {}
};

// =============================
// Statements
// =============================

struct Stmt {
    virtual ~Stmt() = default;
};

struct VarDeclStmt : public Stmt {
    Type* declaredType;
    std::string name;
    std::unique_ptr<Expr> initializer;
    VarDeclStmt(Type* t, const std::string& n, std::unique_ptr<Expr> init)
        : declaredType(t), name(n), initializer(std::move(init)) {}
};

struct AssignmentStmt : public Stmt {
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> value;
    AssignmentStmt(std::unique_ptr<Expr> t, std::unique_ptr<Expr> v)
        : target(std::move(t)), value(std::move(v)) {}
};

struct PrintStmt : public Stmt {
    std::unique_ptr<Expr> expression;
    PrintStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
};

struct LoopStmt : public Stmt {
    std::string iterator;
    std::unique_ptr<Expr> count;
    std::vector<std::unique_ptr<Stmt>> body;
    LoopStmt(const std::string& it, std::unique_ptr<Expr> c)
        : iterator(it), count(std::move(c)) {}
};

struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Stmt>> thenBranch;
    std::vector<std::unique_ptr<Stmt>> elseBranch;
    IfStmt(std::unique_ptr<Expr> cond) : condition(std::move(cond)) {}
};

struct ReturnStmt : public Stmt {
    std::unique_ptr<Expr> value;
    ReturnStmt(std::unique_ptr<Expr> val) : value(std::move(val)) {}
};

struct FunctionDecl : public Stmt {
    std::string name;
    std::vector<std::pair<std::string, Type*>> params;
    Type* returnType;
    std::vector<std::unique_ptr<Stmt>> body;
    FunctionDecl(const std::string& n, Type* ret) : name(n), returnType(ret) {}
};

struct ConstructorDecl {
    std::vector<std::pair<std::string, Type*>> params;
    std::vector<std::unique_ptr<Stmt>> body;
};

struct StructDecl : public Stmt
{
    std::string name;
    std::vector<std::pair<std::string, Type*>> fields;
    std::unique_ptr<ConstructorDecl>  constructor;
    StructDecl(const std::string& n) : name(n) {}
};

struct SelfExpr : public Expr {
    SelfExpr() {}
};

struct SelfAssignmentStmt : public Stmt {
    std::string field;
    std::unique_ptr<Expr> value;
    SelfAssignmentStmt(const std:: string& f, std::unique_ptr<Expr> v)
        : field(f), value(std::move(v)) {}
};

struct ConstructorCallExpr : public Expr {
    std::string structName;
    std::vector<std::unique_ptr<Expr>> arguments;
    ConstructorCallExpr(const std::string& name, std::vector<std::unique_ptr<Expr>> args)
        : structName(name), arguments(std::move(args)) {}
};

struct MemberAccessExpr : public Expr 
{
    std::unique_ptr<Expr> object;
    std::string           field;
    MemberAccessExpr(std::unique_ptr<Expr> obj, const std::string& f)
        :object(std::move(obj)), field(f) {}
};

struct StructLiteralExpr : public Expr 
{
    std::string structName;

    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields;
    StructLiteralExpr(const std::string& name) : structName(name) {}
};

struct Program {
    std::vector<std::unique_ptr<Stmt>> statements;
};

} // namespace nexa