#include "Lexer.h"
#include <cctype>

using namespace nexa;

Lexer::Lexer(const std::string& src)
    : source(src), position(0), line(1), column(1) {}

char Lexer::peek() const {
    if (position >= source.size()) return '\0';
    return source[position];
}

char Lexer::peekNext() const {
    if (position + 1 >= source.size()) return '\0';
    return source[position + 1];
}

char Lexer::advance() {
    char c = peek();
    position++;

    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }

    return c;
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();

        // Skip whitespace
        if (isspace(c)) {
            advance();
        }

        // Skip line comments
        else if (c == '/' && peekNext() == '/') {
            while (peek() != '\n' && peek() != '\0')
                advance();
        }

        else {
            break;
        }
    }
}

Token Lexer::makeToken(TokenKind kind,
                       const std::string& lexeme) {
    return Token{kind, lexeme, line, column};
}

Token Lexer::identifier() {
    std::string lexeme;

    while (isalnum(peek()) || peek() == '_')
        lexeme += advance();

    if (lexeme == "int") return makeToken(TokenKind::Int, lexeme);
    if (lexeme == "double") return makeToken(TokenKind::Double, lexeme);
    if (lexeme == "string") return makeToken(TokenKind::String, lexeme);
    if (lexeme == "print") return makeToken(TokenKind::Print, lexeme);
    if (lexeme == "loop") return makeToken(TokenKind::Loop, lexeme);

    return makeToken(TokenKind::Identifier, lexeme);
}

Token Lexer::number() {
    std::string lexeme;
    bool isDouble = false;

    while (isdigit(peek()))
        lexeme += advance();

    if (peek() == '.') {
        isDouble = true;
        lexeme += advance();
        while (isdigit(peek()))
            lexeme += advance();
    }

    if (isDouble)
        return makeToken(TokenKind::FloatLiteral, lexeme);

    return makeToken(TokenKind::IntegerLiteral, lexeme);
}

Token Lexer::stringLiteral() {
    std::string lexeme;
    advance();

    while (peek() != '"' && peek() != '\0')
        lexeme += advance();

    advance();
    return makeToken(TokenKind::StringLiteral, lexeme);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (peek() != '\0') {

        skipWhitespace();
        char c = peek();

        if (isalpha(c) || c == '_') {
            tokens.push_back(identifier());
        }
        else if (isdigit(c)) {
            tokens.push_back(number());
        }
        else {
            switch (c) {
                case '+': advance(); tokens.push_back(makeToken(TokenKind::Plus, "+")); break;
                case '-': advance(); tokens.push_back(makeToken(TokenKind::Minus, "-")); break;
                case '*': advance(); tokens.push_back(makeToken(TokenKind::Star, "*")); break;
                case '/': advance(); tokens.push_back(makeToken(TokenKind::Slash, "/")); break;
                case '=': advance(); tokens.push_back(makeToken(TokenKind::Assign, "=")); break;
                case '(': advance(); tokens.push_back(makeToken(TokenKind::LeftParen, "(")); break;
                case ')': advance(); tokens.push_back(makeToken(TokenKind::RightParen, ")")); break;
                case '{': advance(); tokens.push_back(makeToken(TokenKind::LeftBrace, "{")); break;
                case '}': advance(); tokens.push_back(makeToken(TokenKind::RightBrace, "}")); break;
                case '[': advance(); tokens.push_back(makeToken(TokenKind::LeftBracket, "[")); break;
                case ']': advance(); tokens.push_back(makeToken(TokenKind::RightBracket, "]")); break;
                case ',': advance(); tokens.push_back(makeToken(TokenKind::Comma, ",")); break;
                case ';': advance(); tokens.push_back(makeToken(TokenKind::Semicolon, ";")); break;
                case '"': tokens.push_back(stringLiteral()); break;
                default:
                    advance();
                    break;
            }
        }
    }

    tokens.push_back(makeToken(TokenKind::END, ""));
    return tokens;
}