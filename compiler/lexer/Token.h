#pragma once
#include <string>

enum class TokenKind {
    // Type symbols
    TYPE_I32_OPEN,     // <
    TYPE_I32_CLOSE,    // >
    TYPE_I64_OPEN,     // <<
    TYPE_I64_CLOSE,    // >>
    TYPE_CHAR_OPEN,    // {
    TYPE_CHAR_CLOSE,   // }
    TYPE_STRING_OPEN,  // {{
    TYPE_STRING_CLOSE, // }}
    TYPE_BOOL,         // /

    // Literals
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    CHAR_LITERAL,
    BOOL_LITERAL,

    // Operators / punctuation
    ASSIGN,    // =
    COMMA,     // ,

    END_OF_FILE,
    INVALID
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    int line;
    int column;
};
