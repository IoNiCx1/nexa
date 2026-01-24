#pragma once
#include "Token.h"
#include <string>

class Lexer {
public:
    explicit Lexer(const std::string& source);

    Token nextToken();

private:
    std::string src;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    // Core helpers
    char peek(int offset = 0) const;
    char advance();
    bool isAtEnd() const;

    void skipWhitespace();

    // Token creation
    Token makeToken(TokenKind kind, const std::string& lexeme);

    // Literal scanners
    Token identifierOrKeyword();
    Token numberLiteral();
    Token stringLiteral();
    Token charLiteral();
};
