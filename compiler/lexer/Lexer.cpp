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

    // Existing keywords
    if (lexeme == "int") return makeToken(TokenKind::Int, lexeme);
    if (lexeme == "double") return makeToken(TokenKind::Double, lexeme);
    if (lexeme == "string") return makeToken(TokenKind::String, lexeme);
    if (lexeme == "print") return makeToken(TokenKind::Print, lexeme);
    if (lexeme == "loop") return makeToken(TokenKind::Loop, lexeme);
    if (lexeme == "if") return makeToken(TokenKind::If, lexeme);
    if (lexeme == "else") return makeToken(TokenKind::Else, lexeme);
    if (lexeme == "true") return makeToken(TokenKind::True, lexeme);
    if (lexeme == "false") return makeToken(TokenKind::False, lexeme);
    if (lexeme == "bool") return makeToken(TokenKind::Bool, lexeme);
    

    // NEW function keywords
    if (lexeme == "fn") return makeToken(TokenKind::Fn, lexeme);
    if (lexeme == "return") return makeToken(TokenKind::Return, lexeme);

    // =============================
    // NEW AI KEYWORD
    // =============================

    if (lexeme == "tensor") return makeToken(TokenKind::Tensor, lexeme);
    if (lexeme == "struct") return makeToken(TokenKind::Struct, lexeme);

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

    while (true) {

        skipWhitespace();

        char c = peek();

        if (c == '\0')
            break;

        // Identifier or keyword
        if (isalpha(c) || c == '_') {
            tokens.push_back(identifier());
            continue;
        }

        // Number
        if (isdigit(c)) {
            tokens.push_back(number());
            continue;
        }

        // String
        if (c == '"') {
            tokens.push_back(stringLiteral());
            continue;
        }

        // Operators & punctuation
        switch (c) {

            // Arithmetic
            case '+':
                advance();
                tokens.push_back(makeToken(TokenKind::Plus, "+"));
                break;
            
            case '.':
                advance();
                tokens.push_back(makeToken(TokenKind::Dot, "."));
                break;

            case '-':
                advance();
                if (peek() == '>')
                {
                    advance();
                    tokens.push_back(makeToken(TokenKind::Arrow, "->"));
                }
                else
                {
                    tokens.push_back(makeToken(TokenKind::Minus, "-"));
                }
                break;

            case '*':
                advance();
                tokens.push_back(makeToken(TokenKind::Star, "*"));
                break;

            case '/':
                advance();
                tokens.push_back(makeToken(TokenKind::Slash, "/"));
                break;

            // Assignment / Equality
            case '=':
                advance();
                if (peek() == '=') {
                    advance();
                    tokens.push_back(makeToken(TokenKind::EqualEqual, "=="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Assign, "="));
                }
                break;

            case '!':
                advance();
                if (peek() == '=') {
                    advance();
                    tokens.push_back(makeToken(TokenKind::NotEqual, "!="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Invalid, "!"));
                }
                break;

            // Comparisons
            case '<':
                advance();
                if (peek() == '=') {
                    advance();
                    tokens.push_back(makeToken(TokenKind::LessEqual, "<="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Less, "<"));
                }
                break;

            case '>':
                advance();
                if (peek() == '=') {
                    advance();
                    tokens.push_back(makeToken(TokenKind::GreaterEqual, ">="));
                } else {
                    tokens.push_back(makeToken(TokenKind::Greater, ">"));
                }
                break;

            // Punctuation
            case '(':
                advance();
                tokens.push_back(makeToken(TokenKind::LeftParen, "("));
                break;

            case ')':
                advance();
                tokens.push_back(makeToken(TokenKind::RightParen, ")"));
                break;

            case '{':
                advance();
                tokens.push_back(makeToken(TokenKind::LeftBrace, "{"));
                break;

            case '}':
                advance();
                tokens.push_back(makeToken(TokenKind::RightBrace, "}"));
                break;

            case '[':
                advance();
                tokens.push_back(makeToken(TokenKind::LeftBracket, "["));
                break;

            case ']':
                advance();
                tokens.push_back(makeToken(TokenKind::RightBracket, "]"));
                break;

            case ',':
                advance();
                tokens.push_back(makeToken(TokenKind::Comma, ","));
                break;

            case ';':
                advance();
                tokens.push_back(makeToken(TokenKind::Semicolon, ";"));
                break;

            case ':':
                advance();
                tokens.push_back(makeToken(TokenKind::Colon, ":"));
                break;

            default:
                advance();
                tokens.push_back(makeToken(TokenKind::Invalid, std::string(1, c)));
                break;
        }
    }

    tokens.push_back(makeToken(TokenKind::END, ""));
    return tokens;
}
