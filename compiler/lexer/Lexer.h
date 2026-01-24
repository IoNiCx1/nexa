#pragma once
#include "Token.h"
#include <string>

class Lexer {
public:
    explicit Lexer(const std::string& source);

    Token nextToken();

private:
    const std::string& src;
    size_t pos = 0;
    int line = 1;
    int column = 1;

    char peek() const;
    char advance();
    void skipWhitespace();
};
