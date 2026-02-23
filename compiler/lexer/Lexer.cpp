#include "Lexer.h"
#include <cctype>

Lexer::Lexer(const std::string &src)
    : source(src), current(0), line(1), column(1) {}

std::vector<Token> Lexer::tokenize() {

  std::vector<Token> tokens;

  while (!isAtEnd()) {

    skipWhitespace();

    if (isAtEnd())
      break;

    size_t tokenStart = current;
    char c = advance();

    switch (c) {

    case '+':
      tokens.push_back({TokenKind::Plus, "+", line, column});
      break;
    case '-':
      tokens.push_back({TokenKind::Minus, "-", line, column});
      break;
    case '*':
      tokens.push_back({TokenKind::Star, "*", line, column});
      break;
    case '/':
      tokens.push_back({TokenKind::Slash, "/", line, column});
      break;
    case '=':
      tokens.push_back({TokenKind::Assign, "=", line, column});
      break;
    case '(':
      tokens.push_back({TokenKind::LeftParen, "(", line, column});
      break;
    case ')':
      tokens.push_back({TokenKind::RightParen, ")", line, column});
      break;
    case ';':
      tokens.push_back({TokenKind::Semicolon, ";", line, column});
      break;

    case '"': {
      size_t start = current;
      while (!isAtEnd() && peek() != '"')
        advance();

      if (isAtEnd()) {
        tokens.push_back({TokenKind::Invalid, "", line, column});
        break;
      }

      std::string text = source.substr(start, current - start);

      advance(); // closing quote

      tokens.push_back({TokenKind::StringLiteral, text, line, column});
      break;
    }

    default:
      if (std::isdigit(c)) {
        size_t start = tokenStart;

        while (!isAtEnd() && std::isdigit(peek()))
          advance();

        bool isFloat = false;

        if (!isAtEnd() && peek() == '.') {
          isFloat = true;
          advance();
          while (!isAtEnd() && std::isdigit(peek()))
            advance();
        }

        std::string text = source.substr(start, current - start);

        tokens.push_back(
            {isFloat ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral,
             text, line, column});
      } else if (std::isalpha(c) || c == '_') {

        size_t start = tokenStart;

        while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_'))
          advance();

        std::string text = source.substr(start, current - start);

        if (text == "int")
          tokens.push_back({TokenKind::Int, text, line, column});
        else if (text == "double")
          tokens.push_back({TokenKind::Double, text, line, column});
        else if (text == "string")
          tokens.push_back({TokenKind::String, text, line, column});
        else if (text == "print")
          tokens.push_back({TokenKind::Print, text, line, column});
        else
          tokens.push_back({TokenKind::Identifier, text, line, column});
      } else {
        tokens.push_back({TokenKind::Invalid, std::string(1, c), line, column});
      }
      break;
    }
  }

  tokens.push_back({TokenKind::END, "", line, column});
  return tokens;
}

char Lexer::advance() {
  column++;
  return source[current++];
}

char Lexer::peek() {
  if (isAtEnd())
    return '\0';
  return source[current];
}

bool Lexer::isAtEnd() { return current >= source.length(); }

void Lexer::skipWhitespace() {
  while (!isAtEnd()) {
    char c = peek();
    if (c == ' ' || c == '\r' || c == '\t') {
      advance();
    } else if (c == '\n') {
      line++;
      column = 1;
      advance();
    } else {
      break;
    }
  }
}