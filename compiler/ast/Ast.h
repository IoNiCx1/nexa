#pragma once
#include <string>
#include <vector>
#include <memory>

/*
  Base AST node
*/
struct ASTNode {
    virtual ~ASTNode() = default;
};

/*
  =========================
  TYPE SYSTEM (SYMBOL BASED)
  =========================
*/
enum class TypeKind {
    I32,     // < >
    I64,     // << >>
    F32,     // < <float> >
    F64,     // << <double> >>
    CHAR,    // { }
    STRING,  // {{ }}
    BOOL     // / /
};

struct TypeSpec {
    TypeKind kind;

    // Shape info (vector / matrix)
    int rows = 1;
    int cols = 1;

    bool isScalar() const { return rows == 1 && cols == 1; }
    bool isVector() const { return rows == 1 && cols > 1; }
    bool isMatrix() const { return rows > 1 && cols > 1; }
};

/*
  =========================
  EXPRESSIONS
  =========================
*/
struct Expr : ASTNode {
    TypeSpec type;
};

struct IntLiteral : Expr {
    int value;
};

struct FloatLiteral : Expr {
    double value;
};

struct BoolLiteral : Expr {
    bool value;
};

struct CharLiteral : Expr {
    char value;
};

struct StringLiteral : Expr {
    std::string value;
};

struct VariableRef : Expr {
    std::string name;
};

/*
  =========================
  DECLARATIONS
  <a,b,c> = 1,2,3
  =========================
*/
struct Declaration : ASTNode {
    TypeSpec type;
    std::vector<std::string> names;
    std::vector<std::unique_ptr<Expr>> values;
};

/*
  =========================
  PROGRAM ROOT
  =========================
*/
struct Program : ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
};
