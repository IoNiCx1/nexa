#ifndef NEXA_LEXER_H
#define NEXA_LEXER_H

#include "Token.h"
#include <string>
#include <vector>

namespace nexa {

class Lexer {
public:
    Lexer(const std::string& source);

    std::vector<Token> tokenize();

private:
    std::string source;
    size_t position;
    int line;
    int column;

    char peek() const;
    char peekNext() const;
    char advance();

    void skipWhitespace();

    Token makeToken(TokenKind kind,
                    const std::string& lexeme);

    Token number();
    Token identifier();
    Token stringLiteral();
};

}

#endif