#ifndef NEXA_SEMANTIC_ANALYZER_H
#define NEXA_SEMANTIC_ANALYZER_H

#include "../ast/Ast.h"
#include "../sema/Type.h"
#include <map>
#include <string>

namespace nexa {

class SemanticAnalyzer {
public:
  bool analyze(Program &program);

private:
  std::map<std::string, Type> symbolTable;

  Type analyzeExpr(Expr *expr);
  bool analyzeStmt(Stmt *stmt);

  Type promoteNumeric(const Type &left, const Type &right);
};

} // namespace nexa

#endif