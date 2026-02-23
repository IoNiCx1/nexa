#ifndef NEXA_PARSER_H
#define NEXA_PARSER_H

#include "../ast/Ast.h"
#include "../lexer/Token.h"
#include <memory>
#include <vector>

namespace nexa {

class Parser {
public:
  Parser(const std::vector<Token> &tokens);

  std::unique_ptr<Program> parseProgram();

private:
  const std::vector<Token> &tokens;
  size_t current;

  bool match(TokenKind kind);
  bool check(TokenKind kind);
  Token advance();
  Token peek();
  Token previous();
  bool isAtEnd();

  std::unique_ptr<Stmt> parseStatement();
  std::unique_ptr<Stmt> parseVarDecl();
  std::unique_ptr<Stmt> parsePrint();

  std::unique_ptr<Expr> parseExpression(int precedence = 0);
  std::unique_ptr<Expr> parsePrimary();

  int getPrecedence(TokenKind kind);
};

} // namespace nexa

#endif