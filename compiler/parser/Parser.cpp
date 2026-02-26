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

    if (match(TokenKind::Print))
        return parsePrint();

    if (match(TokenKind::Loop))
        return parseLoop();

    if (check(TokenKind::Int) ||
        check(TokenKind::Double) ||
        check(TokenKind::String) ||
        check(TokenKind::Bool)){ //added this

        return parseVarDecl();
    }

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

    if (!check(TokenKind::Identifier)) {
        std::cerr << "Expected variable name\n";
        exit(1);
    }

    Token name = advance();

    if (!match(TokenKind::Assign)) {
        std::cerr << "Expected '=' after variable name\n";
        exit(1);
    }

    auto initializer = parseExpression();

    if (!match(TokenKind::Semicolon)) {
        std::cerr << "Expected ';' after declaration\n";
        exit(1);
    }

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
    else if (t.lexeme == "bool") base = &TYPE_BOOL;
    else {
        std::cerr << "Unknown type\n";
        exit(1);
    }

    // Strict array type handling
    if (match(TokenKind::LeftBracket)) {

        if (!match(TokenKind::RightBracket)) {
            std::cerr << "Expected ']' after '[' in array type\n";
            exit(1);
        }

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

    if (!match(TokenKind::Semicolon)) {
        std::cerr << "Expected ';' after assignment\n";
        exit(1);
    }

    return std::make_unique<AssignmentStmt>(
        std::move(target),
        std::move(value)
    );
}

// =============================
// Print
// =============================

std::unique_ptr<Stmt> Parser::parsePrint() {

    if (!match(TokenKind::LeftParen)) {
        std::cerr << "Expected '(' after print\n";
        exit(1);
    }

    auto expr = parseExpression();

    if (!match(TokenKind::RightParen)) {
        std::cerr << "Expected ')' after print expression\n";
        exit(1);
    }

    if (!match(TokenKind::Semicolon)) {
        std::cerr << "Expected ';' after print statement\n";
        exit(1);
    }

    return std::make_unique<PrintStmt>(std::move(expr));
}

// =============================
// Loop
// =============================

std::unique_ptr<Stmt> Parser::parseLoop() {

    if (!match(TokenKind::LeftParen)) {
        std::cerr << "Expected '(' after loop\n";
        exit(1);
    }

    if (!check(TokenKind::Identifier)) {
        std::cerr << "Expected iterator name in loop\n";
        exit(1);
    }

    Token iterator = advance();

    if (!match(TokenKind::Comma)) {
        std::cerr << "Expected ',' in loop\n";
        exit(1);
    }

    auto count = parseExpression();

    if (!match(TokenKind::RightParen)) {
        std::cerr << "Expected ')' after loop parameters\n";
        exit(1);
    }

    if (!match(TokenKind::LeftBrace)) {
        std::cerr << "Expected '{' after loop\n";
        exit(1);
    }

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

    if (t.kind == TokenKind::BooleanLiteral)
    {
        return std::make_unique<BooleanLiteral>(
            t.lexeme == "true"
        );
    }

    // if (t.kind == TokenKind::True)
    //     return std::make_unique<BooleanLiteral>(true);
    
    // if (t.kind == TokenKind::False)
    //     return std::make_unique<BooleanLiteral>(false);

    if (t.kind == TokenKind::Identifier) {
        auto expr =
            std::make_unique<VariableExpr>(t.lexeme);

        return parsePostfix(std::move(expr));
    }

    if (t.kind == TokenKind::LeftParen) {
        auto expr = parseExpression();

        if (!match(TokenKind::RightParen)) {
            std::cerr << "Expected ')'\n";
            exit(1);
        }

        return expr;
    }

    // Array Literal
    if (t.kind == TokenKind::LeftBracket) {

        auto array = std::make_unique<ArrayLiteralExpr>();

        if (!check(TokenKind::RightBracket)) {
            do {
                array->elements.push_back(
                    parseExpression()
                );
            } while (match(TokenKind::Comma));
        }

        if (!match(TokenKind::RightBracket)) {
            std::cerr << "Expected ']' to close array literal\n";
            exit(1);
        }

        return array;
    }

    std::cerr << "Unexpected expression\n";
    exit(1);
}

std::unique_ptr<Expr> Parser::parsePostfix(
    std::unique_ptr<Expr> expr) {

    while (true) {

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

            if (!match(TokenKind::RightParen)) {
                std::cerr << "Expected ')' after function call\n";
                exit(1);
            }

            expr = std::move(call);
        }

        else if (match(TokenKind::LeftBracket)) {

            auto index = parseExpression();

            if (!match(TokenKind::RightBracket)) {
                std::cerr << "Expected ']' after index expression\n";
                exit(1);
            }

            expr = std::make_unique<IndexExpr>(
                std::move(expr),
                std::move(index)
            );
        }

        else break;
    }

    return expr;
}