#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "../sema/Type.h"

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

// ── Struct / OOP expressions ───────────────────

struct SelfExpr : public Expr {
    SelfExpr() {}
};

struct MemberAccessExpr : public Expr {
    std::unique_ptr<Expr> object;
    std::string           field;
    MemberAccessExpr(std::unique_ptr<Expr> obj, const std::string& f)
        : object(std::move(obj)), field(f) {}
};

struct ConstructorCallExpr : public Expr {
    std::string structName;
    std::vector<std::unique_ptr<Expr>> arguments;
    ConstructorCallExpr(const std::string& name, std::vector<std::unique_ptr<Expr>> args)
        : structName(name), arguments(std::move(args)) {}
};

struct StructLiteralExpr : public Expr {
    std::string structName;
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> fields;
    StructLiteralExpr(const std::string& name) : structName(name) {}
};

// ── File handling expressions ──────────────────

enum class FileMode {
    Read,    // open.file("path")
    Write,   // open.file("path", write, "content")
    Append   // open.file("path", append, "content")
};

// open.file("path")                        → read  → returns string
// open.file("path", write,  expr)          → write → returns void
// open.file("path", append, expr)          → append→ returns void
struct FileExpr : public Expr {
    std::unique_ptr<Expr> path;       // string expression for the file path
    FileMode              mode;
    std::unique_ptr<Expr> content;    // only set for Write / Append
    bool                  compileTime; // true when path is a string literal (embed at compile time)

    FileExpr(std::unique_ptr<Expr> p, FileMode m,
             std::unique_ptr<Expr> c, bool ct)
        : path(std::move(p)), mode(m), content(std::move(c)), compileTime(ct) {}
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

struct StructDecl : public Stmt {
    std::string name;
    std::vector<std::pair<std::string, Type*>> fields;
    std::unique_ptr<ConstructorDecl> constructor;
    StructDecl(const std::string& n) : name(n) {}
};

struct SelfAssignmentStmt : public Stmt {
    std::string field;
    std::unique_ptr<Expr> value;
    SelfAssignmentStmt(const std::string& f, std::unique_ptr<Expr> v)
        : field(f), value(std::move(v)) {}
};

// ── File handling statements ───────────────────

// imp open.file()  — declares that this program uses the file module.
// At compile time: no-op in IR (the runtime is linked automatically).
// At parse time: sets a flag so open.file() calls are legal.
struct ImpStmt : public Stmt {
    std::string module;           // e.g. "open.file"
    ImpStmt(const std::string& m) : module(m) {}
};

// Standalone open.file(...) used as a statement (write / append)
struct FileStmt : public Stmt {
    std::unique_ptr<FileExpr> expr;
    FileStmt(std::unique_ptr<FileExpr> e) : expr(std::move(e)) {}
};

struct Program {
    std::vector<std::unique_ptr<Stmt>> statements;
};

} // namespace nexa