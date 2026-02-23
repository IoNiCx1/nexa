#ifndef NEXA_AST_H
#define NEXA_AST_H

#include "../sema/Type.h"
#include <memory>
#include <string>
#include <vector>

namespace nexa {

class ASTNode {
public:
  virtual ~ASTNode() = default;
};

/* ============================
   EXPRESSIONS
   ============================ */

class Expr : public ASTNode {
public:
  Type inferredType; // Filled by Semantic Analyzer
  virtual ~Expr() = default;
};

class IntegerLiteral : public Expr {
public:
  int value;
  explicit IntegerLiteral(int v) : value(v) {}
};

class DoubleLiteral : public Expr {
public:
  double value;
  explicit DoubleLiteral(double v) : value(v) {}
};

class StringLiteral : public Expr {
public:
  std::string value;
  explicit StringLiteral(const std::string &v) : value(v) {}
};

class VariableExpr : public Expr {
public:
  std::string name;
  explicit VariableExpr(const std::string &n) : name(n) {}
};

class UnaryExpr : public Expr {
public:
  std::string op;
  std::unique_ptr<Expr> operand;

  UnaryExpr(const std::string &o, std::unique_ptr<Expr> expr)
      : op(o), operand(std::move(expr)) {}
};

class BinaryExpr : public Expr {
public:
  std::string op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  BinaryExpr(std::unique_ptr<Expr> l, const std::string &o,
             std::unique_ptr<Expr> r)
      : op(o), left(std::move(l)), right(std::move(r)) {}
};

/* ============================
   STATEMENTS
   ============================ */

class Stmt : public ASTNode {
public:
  virtual ~Stmt() = default;
};

class VarDeclStmt : public Stmt {
public:
  Type declaredType;
  std::string name;
  std::unique_ptr<Expr> initializer;

  VarDeclStmt(const Type &type, const std::string &n,
              std::unique_ptr<Expr> init)
      : declaredType(type), name(n), initializer(std::move(init)) {}
};

class PrintStmt : public Stmt {
public:
  std::unique_ptr<Expr> expression;

  explicit PrintStmt(std::unique_ptr<Expr> expr)
      : expression(std::move(expr)) {}
};

/* ============================
   PROGRAM ROOT
   ============================ */

class Program : public ASTNode {
public:
  std::vector<std::unique_ptr<Stmt>> statements;
};

} // namespace nexa

#endif