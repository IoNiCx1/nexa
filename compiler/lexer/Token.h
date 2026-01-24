#pragma once
#include <string>

enum class TokenKind {
    // Identifiers & literals
    Identifier,
    Number,
    String,

    // Symbols
    Comma,      // ,
    Assign,     // =
    LAngle,     // <
    RAngle,     // >

    // End of file
    End
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    int line;
    int column;
};
