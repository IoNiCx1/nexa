#include "Lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& source) : src(source) {}

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
    while (true) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            advance();
            line++;
            column = 1;
        } else {
            break;
        }
    }
}

Token Lexer::makeToken(TokenKind kind, const std::string& lexeme) {
    return Token{kind, lexeme, line, column};
}

Token Lexer::nextToken() {
    skipWhitespace();
    char c = peek();

    if (c == '\0') return makeToken(TokenKind::EndOfFile, "");

    // Longest Match
    if (c == '<' && peek(1) == '<') { advance(); advance(); return makeToken(TokenKind::TYPE_I64_OPEN, "<<"); }
    if (c == '>' && peek(1) == '>') { advance(); advance(); return makeToken(TokenKind::TYPE_I64_CLOSE, ">>"); }
    if (c == '{' && peek(1) == '{') { advance(); advance(); return makeToken(TokenKind::TYPE_STRING_OPEN, "{{"); }
    if (c == '}' && peek(1) == '}') { advance(); advance(); return makeToken(TokenKind::TYPE_STRING_CLOSE, "}}"); }

    // Single Characters
    if (c == '=') { advance(); return makeToken(TokenKind::Assign, "="); }
    if (c == ',') { advance(); return makeToken(TokenKind::Comma, ","); }
    if (c == '<') { advance(); return makeToken(TokenKind::TYPE_I32_OPEN, "<"); }
    if (c == '>') { advance(); return makeToken(TokenKind::TYPE_I32_CLOSE, ">"); }
    if (c == '{') { advance(); return makeToken(TokenKind::TYPE_CHAR_OPEN, "{"); }
    if (c == '}') { advance(); return makeToken(TokenKind::TYPE_CHAR_CLOSE, "}"); }
    if (c == '/') { advance(); return makeToken(TokenKind::TYPE_BOOL, "/"); }

    if (std::isdigit(c)) return numberLiteral();
    if (std::isalpha(c) || c == '_') return identifierOrKeyword();
    if (c == '"') return stringLiteral();
    if (c == '\'') return charLiteral();

    return makeToken(TokenKind::Invalid, std::string(1, advance()));
}

Token Lexer::identifierOrKeyword() {
    size_t start = pos;
    while (std::isalnum(peek()) || peek() == '_') advance();
    std::string text = src.substr(start, pos - start);

    if (text == "int")   return makeToken(TokenKind::KeywordInt, text);
    if (text == "float") return makeToken(TokenKind::KeywordFloat, text);
    if (text == "true" || text == "false") return makeToken(TokenKind::BoolLiteral, text);

    return makeToken(TokenKind::Identifier, text);
}

Token Lexer::numberLiteral() {
    size_t start = pos;
    bool isFloat = false;
    while (std::isdigit(peek())) advance();
    if (peek() == '.') {
        isFloat = true;
        advance();
        while (std::isdigit(peek())) advance();
    }
    return makeToken(isFloat ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral, src.substr(start, pos - start));
}

Token Lexer::stringLiteral() {
    advance(); // opening "
    size_t start = pos;
    while (peek() != '"' && peek() != '\0') advance();
    std::string text = src.substr(start, pos - start);
    if (peek() == '"') advance(); 
    return makeToken(TokenKind::StringLiteral, text);
}

Token Lexer::charLiteral() {
    advance(); // '
    std::string val(1, advance());
    if (peek() == '\'') advance();
    return makeToken(TokenKind::CharLiteral, val);
}
