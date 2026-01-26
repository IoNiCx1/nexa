#pragma once
#include <string>

enum class TokenKind {
    EndOfFile,
    Invalid,

    // Keywords
    KeywordInt,
    KeywordFloat,
    KeywordBool,
    Print,
    // Literals
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    BoolLiteral,

    // Operators & Brackets
    Assign,    // =
    Comma,     // ,
    Semicolon, // ;
    
    // Your Custom Type Openers
    TYPE_I64_OPEN, TYPE_I64_CLOSE,     // << >>
    TYPE_I32_OPEN, TYPE_I32_CLOSE,     // < >
    TYPE_STRING_OPEN, TYPE_STRING_CLOSE, // {{ }}
    TYPE_CHAR_OPEN, TYPE_CHAR_CLOSE,   // { }
    TYPE_BOOL                          // /
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    int line;
    int column;
};
