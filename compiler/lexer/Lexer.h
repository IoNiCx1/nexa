#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <vector>

class Lexer {
public:
  Lexer(const std::string &source);

  std::vector<Token> tokenize();

private:
  std::string source;
  size_t current;
  int line;
  int column;

  char advance();
  char peek();
  bool isAtEnd();

  void skipWhitespace();
  Token number();
  Token identifier();
  Token stringLiteral();

  bool isAlpha(char c);
  bool isDigit(char c);
};

#endif