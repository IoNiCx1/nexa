#include "SemanticAnalyzer.h"
#include <iostream>

using namespace nexa;

bool SemanticAnalyzer::analyze(Program &program) {

  for (auto &stmt : program.statements) {
    if (!analyzeStmt(stmt.get()))
      return false;
  }

  return true;
}

bool SemanticAnalyzer::analyzeStmt(Stmt *stmt) {

  if (auto varDecl = dynamic_cast<VarDeclStmt *>(stmt)) {

    if (symbolTable.count(varDecl->name)) {
      std::cerr << "Variable already declared: " << varDecl->name << "\n";
      return false;
    }

    Type initType = analyzeExpr(varDecl->initializer.get());

    if (initType.kind == TypeKind::Invalid)
      return false;

    // Numeric promotion allowed
    if (varDecl->declaredType.isNumeric() && initType.isNumeric()) {

      varDecl->initializer->inferredType =
          promoteNumeric(initType, varDecl->declaredType);
    } else if (varDecl->declaredType != initType) {

      std::cerr << "Type mismatch in declaration of " << varDecl->name << "\n";
      return false;
    }

    symbolTable[varDecl->name] = varDecl->declaredType;

    return true;
  }

  if (auto printStmt = dynamic_cast<PrintStmt *>(stmt)) {

    Type exprType = analyzeExpr(printStmt->expression.get());

    if (exprType.kind == TypeKind::Invalid)
      return false;

    return true;
  }

  return true;
}

Type SemanticAnalyzer::analyzeExpr(Expr *expr) {

  if (auto intLit = dynamic_cast<IntegerLiteral *>(expr)) {

    expr->inferredType = Type::getInt();
    return expr->inferredType;
  }

  if (auto dblLit = dynamic_cast<DoubleLiteral *>(expr)) {

    expr->inferredType = Type::getDouble();
    return expr->inferredType;
  }

  if (auto strLit = dynamic_cast<StringLiteral *>(expr)) {

    expr->inferredType = Type::getString();
    return expr->inferredType;
  }

  if (auto var = dynamic_cast<VariableExpr *>(expr)) {

    if (!symbolTable.count(var->name)) {
      std::cerr << "Undefined variable: " << var->name << "\n";
      return Type::getInvalid();
    }

    expr->inferredType = symbolTable[var->name];

    return expr->inferredType;
  }

  if (auto unary = dynamic_cast<UnaryExpr *>(expr)) {

    Type operandType = analyzeExpr(unary->operand.get());

    if (!operandType.isNumeric()) {
      std::cerr << "Unary operator requires numeric type\n";
      return Type::getInvalid();
    }

    expr->inferredType = operandType;
    return expr->inferredType;
  }

  if (auto binary = dynamic_cast<BinaryExpr *>(expr)) {

    Type left = analyzeExpr(binary->left.get());

    Type right = analyzeExpr(binary->right.get());

    if (!left.isNumeric() || !right.isNumeric()) {
      std::cerr << "Binary operator requires numeric types\n";
      return Type::getInvalid();
    }

    Type result = promoteNumeric(left, right);

    expr->inferredType = result;
    return result;
  }

  return Type::getInvalid();
}

Type SemanticAnalyzer::promoteNumeric(const Type &left, const Type &right) {

  if (left.isDouble() || right.isDouble())
    return Type::getDouble();

  return Type::getInt();
}