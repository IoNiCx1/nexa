#include "Token.h"
#include "Lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& source) : src(source), pos(0), line(1), column(1) {}

char Lexer::peek(int offset) const {
    if (pos + offset >= src.size()) return '\0';
    return src[pos + offset];
}

bool Lexer::isAtEnd() const {
    return pos >= src.size();
}

char Lexer::advance() {
    char c = peek();
    pos++;
    column++;
    return c;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && (peek() == ' ' || peek() == '\r' || peek() == '\t' || peek() == '\n')) {
        if (peek() == '\n') {
            line++;
            column = 1;
        }
        pos++; 
    }
}

Token Lexer::makeToken(TokenKind kind, const std::string& lexeme) {
    return Token{kind, lexeme, line, column - (int)lexeme.length()};
}

Token Lexer::nextToken() {
    skipWhitespace();
    if (isAtEnd()) return makeToken(TokenKind::EndOfFile, "");
    
    char c = peek();
    
    // --- Multi-character check (Longest Match) ---
    if (c == '<' && peek(1) == '<') { advance(); advance(); return makeToken(TokenKind::TYPE_I64_OPEN, "<<"); }
    if (c == '>' && peek(1) == '>') { advance(); advance(); return makeToken(TokenKind::TYPE_I64_CLOSE, ">>"); }
    
    // --- Single Characters ---
    if (c == '=') { advance(); return makeToken(TokenKind::Assign, "="); }
    if (c == ',') { advance(); return makeToken(TokenKind::Comma, ","); }
    if (c == ';') { advance(); return makeToken(TokenKind::Semicolon, ";"); }
    if (c == '.') { advance(); return makeToken(TokenKind::Dot, "."); }
    if (c == '/') { advance(); return makeToken(TokenKind::Slash, "/"); }
    
    // Nexa Matrix Brackets
    if (c == '<') { advance(); return makeToken(TokenKind::TYPE_I32_OPEN, "<"); }
    if (c == '>') { advance(); return makeToken(TokenKind::TYPE_I32_CLOSE, ">"); }
    if (c == '(') { advance(); return makeToken(TokenKind::TYPE_CHAR_OPEN, "("); }
    if (c == ')') { advance(); return makeToken(TokenKind::TYPE_CHAR_CLOSE, ")"); }
    if (c == '{') { advance(); return makeToken(TokenKind::TYPE_STRING_OPEN, "{"); }
    if (c == '}') { advance(); return makeToken(TokenKind::TYPE_STRING_CLOSE, "}"); }
    
    // --- Literals ---
    if (std::isdigit(c)) return numberLiteral();
    if (std::isalpha(c) || c == '_') return identifierOrKeyword();
    if (c == '"') return stringLiteral();
    if (c == '\'') return charLiteral();
    
    return makeToken(TokenKind::Invalid, std::string(1, advance()));
}

Token Lexer::identifierOrKeyword() {
    size_t start = pos;
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) advance();
    std::string text = src.substr(start, pos - start);
    
    if (text == "int")   return makeToken(TokenKind::KeywordInt, text);
    if (text == "print") return makeToken(TokenKind::Print, text);
    if (text == "true" || text == "false") return makeToken(TokenKind::BoolLiteral, text);
    
    return makeToken(TokenKind::Identifier, text);
}

Token Lexer::numberLiteral() {
    size_t start = pos;
    bool isFloat = false;
    while (!isAtEnd() && std::isdigit(peek())) advance();
    if (peek() == '.') {
        isFloat = true;
        advance();
        while (!isAtEnd() && std::isdigit(peek())) advance();
    }
    return makeToken(isFloat ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral, src.substr(start, pos - start));
}

Token Lexer::stringLiteral() {
    advance(); 
    size_t start = pos;
    while (!isAtEnd() && peek() != '"') advance();
    std::string text = src.substr(start, pos - start);
    if (!isAtEnd()) advance(); 
    return makeToken(TokenKind::StringLiteral, text);
}

Token Lexer::charLiteral() {
    advance(); 
    size_t start = pos;
    if (!isAtEnd()) advance();  
    std::string val = src.substr(start, pos - start);
    if (peek() == '\'') advance(); 
    return makeToken(TokenKind::CharLiteral, val);
}
