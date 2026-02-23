#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenKind {

  // Literals
  IntegerLiteral,
  FloatLiteral,
  StringLiteral,
  Identifier,

  // Type Keywords
  Int,
  Double,
  String,

  // Other Keywords
  Print,

  // Operators
  Plus,
  Minus,
  Star,
  Slash,
  Assign,

  // Punctuation
  LeftParen,
  RightParen,
  Semicolon,

  // System
  END,
  Invalid
};

struct Token {
  TokenKind kind;
  std::string lexeme;
  int line;
  int column;
};

#endif