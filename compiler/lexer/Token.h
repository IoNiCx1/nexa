#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenKind {
    // Literals
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    BoolLiteral,
    Identifier,

    // Keywords
    KeywordInt,
    KeywordFloat,
    Print,

    // Operators and Punctuation
    Assign,     // =
    Comma,      // ,
    Semicolon,  // ;
    Dot,        // .
    Slash,      // /

    // Nexa Matrix Brackets
    TYPE_I32_OPEN,    // <
    TYPE_I32_CLOSE,   // >
    TYPE_I64_OPEN,    // <<
    TYPE_I64_CLOSE,   // >>
    TYPE_CHAR_OPEN,   // (
    TYPE_CHAR_CLOSE,  // )
    TYPE_STRING_OPEN, // {
    TYPE_STRING_CLOSE,// }

    // System
    Invalid,
    EndOfFile
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    int line;
    int column;
};

#endif
