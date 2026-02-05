#include "Lexer.h"
#include <cctype>
#include <stdexcept>

/* ================= CONSTRUCTOR ================= */

Lexer::Lexer(const std::string& source)
    : src(source) {}

/* ================= CORE HELPERS ================= */

bool Lexer::isAtEnd() const {
    return pos >= src.size();
}

char Lexer::peek(int offset) const {
    if (pos + offset >= src.size()) return '\0';
    return src[pos + offset];
}

char Lexer::advance() {
    char c = peek();
    pos++;
    column++;
    return c;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
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

/* ================= TOKEN CREATION ================= */

Token Lexer::makeToken(TokenKind kind, const std::string& lexeme) {
    return Token{ kind, lexeme, line, column };
}

/* ================= SCANNERS ================= */

Token Lexer::identifierOrKeyword() {
    std::string value;
    while (std::isalnum(peek()) || peek() == '_') {
        value.push_back(advance());
    }

    if (value == "int")   return makeToken(TokenKind::KeywordInt, value);
    if (value == "print") return makeToken(TokenKind::Print, value);

    return makeToken(TokenKind::Identifier, value);
}

Token Lexer::numberLiteral() {
    std::string value;

    if (peek() == '-') {
        value.push_back(advance());
    }

    while (std::isdigit(peek())) {
        value.push_back(advance());
    }

    return makeToken(TokenKind::IntegerLiteral, value);
}

/* 🔥 FIXED STRING LITERAL (SPACES ALLOWED) */
Token Lexer::stringLiteral() {
    advance(); // consume opening "

    std::string value;
    while (!isAtEnd() && peek() != '"') {
        value.push_back(advance());
    }

    if (isAtEnd()) {
        throw std::runtime_error("Unterminated string literal");
    }

    advance(); // consume closing "

    return makeToken(TokenKind::StringLiteral, value);
}

Token Lexer::charLiteral() {
    advance(); // '

    if (isAtEnd())
        throw std::runtime_error("Unterminated char literal");

    char value = advance();

    if (peek() != '\'')
        throw std::runtime_error("Invalid char literal");

    advance(); // closing '

    return makeToken(TokenKind::CharLiteral, std::string(1, value));
}

/* ================= MAIN LEXER ================= */

Token Lexer::nextToken() {
    skipWhitespace();

    if (isAtEnd())
        return makeToken(TokenKind::EndOfFile, "");

    char c = peek();

    /* literals */
    if (c == '"') return stringLiteral();
    if (c == '\'') return charLiteral();
    if (std::isdigit(c) || (c == '-' && std::isdigit(peek(1))))
        return numberLiteral();

    /* identifiers */
    if (std::isalpha(c) || c == '_')
        return identifierOrKeyword();

    /* single-character tokens */
    advance();
    switch (c) {
        case '=': return makeToken(TokenKind::Assign, "=");
        case ',': return makeToken(TokenKind::Comma, ",");
        case ';': return makeToken(TokenKind::Semicolon, ";");
        case '.': return makeToken(TokenKind::Dot, ".");
        case '/': return makeToken(TokenKind::Slash, "/");

        case '<': return makeToken(TokenKind::TYPE_I32_OPEN, "<");
        case '>': return makeToken(TokenKind::TYPE_I32_CLOSE, ">");
        case '(': return makeToken(TokenKind::TYPE_CHAR_OPEN, "(");
        case ')': return makeToken(TokenKind::TYPE_CHAR_CLOSE, ")");
        case '{': return makeToken(TokenKind::TYPE_STRING_OPEN, "{");
        case '}': return makeToken(TokenKind::TYPE_STRING_CLOSE, "}");
    }

    return makeToken(TokenKind::Invalid, std::string(1, c));
}
