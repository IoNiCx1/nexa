#include "Parser.h"
#include <iostream>

using namespace nexa;

Parser::Parser(const std::vector<Token>& toks)
    : tokens(toks) {}

Token Parser::peek() { return tokens[current]; }
Token Parser::previous() { return tokens[current - 1]; }
bool Parser::isAtEnd() { return peek().kind == TokenKind::END; }

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenKind kind) {
    if (isAtEnd()) return false;
    return peek().kind == kind;
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

// =============================
// Program
// =============================

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (!isAtEnd()) {
        program->statements.push_back(parseStatement());
    }

    return program;
}

// =============================
// Statements
// =============================

std::unique_ptr<Stmt> Parser::parseStatement() {
    // std::cout << "DEBUG TOKEN KIND: "
    //       << static_cast<int>(peek().kind)
    //       << " -> "
    //       << peek().lexeme << "\n";
    // Prints
    if (match(TokenKind::Print))
        return parsePrint();

    // Loop
    if (match(TokenKind::Loop))
        return parseLoop();

    // Variable declarations
    if (check(TokenKind::Int) ||
        check(TokenKind::Double) ||
        check(TokenKind::String)) {

        return parseVarDecl();
    }

    // Assignment
    if (check(TokenKind::Identifier))
        return parseAssignment();

    std::cerr << "Unexpected statement near token: "
              << peek().lexeme << "\n";
    exit(1);
}

// =============================
// Variable Declaration
// =============================

std::unique_ptr<Stmt> Parser::parseVarDecl() {

    Type* declaredType = parseType();

    Token name = advance(); // identifier

    match(TokenKind::Assign);

    auto initializer = parseExpression();

    match(TokenKind::Semicolon);

    return std::make_unique<VarDeclStmt>(
        declaredType,
        name.lexeme,
        std::move(initializer)
    );
}

Type* Parser::parseType() {

    Token t = advance();

    Type* base = nullptr;

    if (t.lexeme == "int") base = &TYPE_INT;
    else if (t.lexeme == "double") base = &TYPE_DOUBLE;
    else if (t.lexeme == "string") base = &TYPE_STRING;
    else {
        std::cerr << "Unknown type\n";
        exit(1);
    }

    // Check for array type
    if (match(TokenKind::LeftBracket)) {
        match(TokenKind::RightBracket);
        return new Type(TypeKind::Array, base);
    }

    return base;
}

// =============================
// Assignment
// =============================

std::unique_ptr<Stmt> Parser::parseAssignment() {

    auto target = parseExpression();

    if (!match(TokenKind::Assign)) {
        std::cerr << "Expected '='\n";
        exit(1);
    }

    auto value = parseExpression();

    match(TokenKind::Semicolon);

    return std::make_unique<AssignmentStmt>(
        std::move(target),
        std::move(value)
    );
}

// =============================
// Print
// =============================

std::unique_ptr<Stmt> Parser::parsePrint() {

    match(TokenKind::LeftParen);

    auto expr = parseExpression();

    match(TokenKind::RightParen);
    match(TokenKind::Semicolon);

    return std::make_unique<PrintStmt>(std::move(expr));
}

// =============================
// Loop
// =============================

std::unique_ptr<Stmt> Parser::parseLoop() {

    match(TokenKind::LeftParen);

    Token iterator = advance();

    match(TokenKind::Comma);

    auto count = parseExpression();

    match(TokenKind::RightParen);
    match(TokenKind::LeftBrace);

    auto loop = std::make_unique<LoopStmt>(
        iterator.lexeme,
        std::move(count)
    );

    while (!check(TokenKind::RightBrace)) {
        loop->body.push_back(parseStatement());
    }

    match(TokenKind::RightBrace);

    return loop;
}

// =============================
// Expressions (Pratt)
// =============================

int Parser::getPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind::Plus:
        case TokenKind::Minus: return 1;
        case TokenKind::Star:
        case TokenKind::Slash: return 2;
        default: return 0;
    }
}

std::unique_ptr<Expr> Parser::parseExpression(int precedence) {

    auto left = parsePrimary();

    while (!isAtEnd() &&
           getPrecedence(peek().kind) > precedence) {

        Token op = advance();
        int newPrec = getPrecedence(op.kind);

        auto right = parseExpression(newPrec);

        left = std::make_unique<BinaryExpr>(
            std::move(left),
            op.lexeme,
            std::move(right)
        );
    }

    return left;
}

std::unique_ptr<Expr> Parser::parsePrimary() {

    Token t = advance();

    if (t.kind == TokenKind::IntegerLiteral)
        return std::make_unique<IntegerLiteral>(
            std::stoi(t.lexeme)
        );

    if (t.kind == TokenKind::FloatLiteral)
        return std::make_unique<DoubleLiteral>(
            std::stod(t.lexeme)
        );

    if (t.kind == TokenKind::StringLiteral)
        return std::make_unique<StringLiteral>(
            t.lexeme
        );

    if (t.kind == TokenKind::Identifier) {

        auto expr =
            std::make_unique<VariableExpr>(t.lexeme);

        return parsePostfix(std::move(expr));
    }

    if (t.kind == TokenKind::LeftParen) {
        auto expr = parseExpression();
        match(TokenKind::RightParen);
        return expr;
    }

    if (t.kind == TokenKind::LeftBracket) {

        auto array = std::make_unique<ArrayLiteralExpr>();

        if (!check(TokenKind::RightBracket)) {
            do {
                array->elements.push_back(
                    parseExpression()
                );
            } while (match(TokenKind::Comma));
        }

        match(TokenKind::RightBracket);
        return array;
    }

    std::cerr << "Unexpected expression\n";
    exit(1);
}

std::unique_ptr<Expr> Parser::parsePostfix(
    std::unique_ptr<Expr> expr) {

    while (true) {

        // Function call
        if (match(TokenKind::LeftParen)) {

            auto call =
                std::make_unique<CallExpr>();

            call->callee =
                dynamic_cast<VariableExpr*>(expr.get())->name;

            if (!check(TokenKind::RightParen)) {
                do {
                    call->arguments.push_back(
                        parseExpression()
                    );
                } while (match(TokenKind::Comma));
            }

            match(TokenKind::RightParen);
            expr = std::move(call);
        }

        // Indexing
        else if (match(TokenKind::LeftBracket)) {

            auto index = parseExpression();
            match(TokenKind::RightBracket);

            expr = std::make_unique<IndexExpr>(
                std::move(expr),
                std::move(index)
            );
        }

        else break;
    }

    return expr;
}