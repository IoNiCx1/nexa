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

    if (c == '\0')
        return makeToken(TokenKind::END_OF_FILE, "");

    // ---- LONGEST MATCH FIRST ----

    if (c == '<' && peek(1) == '<') {
        advance(); advance();
        return makeToken(TokenKind::TYPE_I64_OPEN, "<<");
    }

    if (c == '>' && peek(1) == '>') {
        advance(); advance();
        return makeToken(TokenKind::TYPE_I64_CLOSE, ">>");
    }

    if (c == '{' && peek(1) == '{') {
        advance(); advance();
        return makeToken(TokenKind::TYPE_STRING_OPEN, "{{");
    }

    if (c == '}' && peek(1) == '}') {
        advance(); advance();
        return makeToken(TokenKind::TYPE_STRING_CLOSE, "}}");
    }

    if (c == '<') { advance(); return makeToken(TokenKind::TYPE_I32_OPEN, "<"); }
    if (c == '>') { advance(); return makeToken(TokenKind::TYPE_I32_CLOSE, ">"); }
    if (c == '{') { advance(); return makeToken(TokenKind::TYPE_CHAR_OPEN, "{"); }
    if (c == '}') { advance(); return makeToken(TokenKind::TYPE_CHAR_CLOSE, "}"); }
    if (c == '/') { advance(); return makeToken(TokenKind::TYPE_BOOL, "/"); }

    if (c == '=') { advance(); return makeToken(TokenKind::ASSIGN, "="); }
    if (c == ',') { advance(); return makeToken(TokenKind::COMMA, ","); }

    if (std::isdigit(c))
        return numberLiteral();

    if (std::isalpha(c) || c == '_')
        return identifierOrKeyword();

    if (c == '"')
        return stringLiteral();

    if (c == '\'')
        return charLiteral();

    advance();
    return makeToken(TokenKind::INVALID, std::string(1, c));
}

Token Lexer::identifierOrKeyword() {
    size_t start = pos;
    while (std::isalnum(peek()) || peek() == '_')
        advance();

    std::string text = src.substr(start, pos - start);

    if (text == "true" || text == "false")
        return makeToken(TokenKind::BOOL_LITERAL, text);

    return makeToken(TokenKind::IDENTIFIER, text);
}

Token Lexer::numberLiteral() {
    size_t start = pos;
    bool isFloat = false;

    while (std::isdigit(peek()))
        advance();

    if (peek() == '.') {
        isFloat = true;
        advance();
        while (std::isdigit(peek()))
            advance();
    }

    std::string num = src.substr(start, pos - start);
    return makeToken(isFloat ? TokenKind::FLOAT_LITERAL : TokenKind::INT_LITERAL, num);
}

Token Lexer::stringLiteral() {
    advance(); // "
    size_t start = pos;

    while (peek() != '"' && peek() != '\0')
        advance();

    std::string text = src.substr(start, pos - start);
    advance(); // closing "

    return makeToken(TokenKind::STRING_LITERAL, text);
}

Token Lexer::charLiteral() {
    advance(); // '
    char value = advance();
    advance(); // closing '

    return makeToken(TokenKind::CHAR_LITERAL, std::string(1, value));
}
