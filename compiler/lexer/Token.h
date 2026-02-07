#pragma once
#include <string>

enum class TokenKind {
    /* literals */
    IntegerLiteral,
    StringLiteral,
    CharLiteral,
    Identifier,

    /* keywords */
    KeywordInt,
    Print,

    /* operators */
    Plus,       // +
    Minus,      // -
    Star,       // *
    Slash,      // /
    Percent,    // %

    Assign,     // =
    Comma,      // ,
    Semicolon,  // ;
    Dot,        // .

    /* brackets */
    TYPE_I32_OPEN,    // <
    TYPE_I32_CLOSE,   // >
    TYPE_CHAR_OPEN,   // (
    TYPE_CHAR_CLOSE,  // )
    TYPE_STRING_OPEN, // {
    TYPE_STRING_CLOSE,// }

    EndOfFile,
    Invalid
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    int line;
    int column;
};
