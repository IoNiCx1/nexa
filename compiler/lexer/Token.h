#ifndef TOKEN_H
#define TOKEN_H

#include <string>

namespace nexa {

enum class TokenKind {

    // Literals
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    Identifier,

    // Keywords
    Int,
    Double,
    String,
    Bool,
    Print,
    Loop,
    If,
    Else,
    True,
    False,

    // Function keywords
    Fn,
    Return,

    // Operators
    Plus,
    Minus,
    Star,
    Slash,
    Assign,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    EqualEqual,
    NotEqual,
    Tensor,

    // Punctuation
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Comma,
    Semicolon,

    END,
    Invalid
};

struct Token {
    TokenKind kind;
    std::string lexeme;
    int line;
    int column;
};

}

#endif