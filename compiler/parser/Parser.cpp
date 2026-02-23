#include "Parser.h"
#include <iostream>

using namespace nexa;

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

std::unique_ptr<Program> Parser::parseProgram() {
  auto program = std::make_unique<Program>();

  while (true) {

    if (peek().kind == TokenKind::END)
      break;

    auto stmt = parseStatement();

    if (!stmt) {
      std::cerr << "Parser stuck at token: " << peek().lexeme << "\n";
      break; // STOP immediately
    }

    program->statements.push_back(std::move(stmt));
  }

  return program;
}

std::unique_ptr<Stmt> Parser::parseStatement() {

  if (peek().kind == TokenKind::Int || peek().kind == TokenKind::Double ||
      peek().kind == TokenKind::String) {

    return parseVarDecl();
  }

  if (peek().kind == TokenKind::Print) {
    advance(); // consume print
    return parsePrint();
  }

  std::cerr << "Unexpected statement near token: " << peek().lexeme << "\n";

  return nullptr;
}

std::unique_ptr<Stmt> Parser::parseVarDecl() {

  Token typeToken = advance(); // consume type

  Type declaredType;

  if (typeToken.kind == TokenKind::Int)
    declaredType = Type::getInt();
  else if (typeToken.kind == TokenKind::Double)
    declaredType = Type::getDouble();
  else if (typeToken.kind == TokenKind::String)
    declaredType = Type::getString();
  else {
    std::cerr << "Invalid type in declaration\n";
    return nullptr;
  }

  Token name = advance();

  if (name.kind != TokenKind::Identifier) {
    std::cerr << "Expected variable name\n";
    return nullptr;
  }

  if (!match(TokenKind::Assign)) {
    std::cerr << "Expected '=' after variable name\n";
    return nullptr;
  }

  auto initializer = parseExpression();

  if (!match(TokenKind::Semicolon)) {
    std::cerr << "Expected ';' after declaration\n";
    return nullptr;
  }

  return std::make_unique<VarDeclStmt>(declaredType, name.lexeme,
                                       std::move(initializer));
}

std::unique_ptr<Stmt> Parser::parsePrint() {

  if (!match(TokenKind::LeftParen)) {
    std::cerr << "Expected '(' after print\n";
    return nullptr;
  }

  auto expr = parseExpression();

  if (!match(TokenKind::RightParen)) {
    std::cerr << "Expected ')' after expression\n";
    return nullptr;
  }

  if (!match(TokenKind::Semicolon)) {
    std::cerr << "Expected ';' after print\n";
    return nullptr;
  }

  return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::parseExpression(int precedence) {

  auto left = parsePrimary();

  while (!isAtEnd() && precedence < getPrecedence(peek().kind)) {

    Token op = advance();
    int opPrec = getPrecedence(op.kind);

    auto right = parseExpression(opPrec);

    left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme,
                                        std::move(right));
  }

  return left;
}

std::unique_ptr<Expr> Parser::parsePrimary() {

  Token token = advance();

  if (token.kind == TokenKind::IntegerLiteral) {
    return std::make_unique<IntegerLiteral>(std::stoi(token.lexeme));
  }

  if (token.kind == TokenKind::FloatLiteral) {
    return std::make_unique<DoubleLiteral>(std::stod(token.lexeme));
  }

  if (token.kind == TokenKind::StringLiteral) {
    return std::make_unique<StringLiteral>(token.lexeme);
  }

  if (token.kind == TokenKind::Identifier) {
    return std::make_unique<VariableExpr>(token.lexeme);
  }

  if (token.kind == TokenKind::Minus) {

    auto operand = parseExpression(20);

    return std::make_unique<UnaryExpr>("-", std::move(operand));
  }

  if (token.kind == TokenKind::LeftParen) {

    auto expr = parseExpression();

    if (!match(TokenKind::RightParen)) {
      std::cerr << "Expected ')'\n";
      return nullptr;
    }

    return expr;
  }

  std::cerr << "Unexpected expression\n";
  return nullptr;
}

int Parser::getPrecedence(TokenKind kind) {

  switch (kind) {
  case TokenKind::Star:
  case TokenKind::Slash:
    return 20;

  case TokenKind::Plus:
  case TokenKind::Minus:
    return 10;

  default:
    return 0;
  }
}

bool Parser::match(TokenKind kind) {
  if (check(kind)) {
    advance();
    return true;
  }
  return false;
}

bool Parser::check(TokenKind kind) {
  if (isAtEnd())
    return false;
  return peek().kind == kind;
}

Token Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}

bool Parser::isAtEnd() { return peek().kind == TokenKind::END; }

Token Parser::peek() { return tokens[current]; }

Token Parser::previous() { return tokens[current - 1]; }