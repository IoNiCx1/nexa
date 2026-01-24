#include "Parser.h"
#include <stdexcept>

/* =========================
   Constructor
   ========================= */

Parser::Parser(Lexer& lexer)
    : lexer(lexer) {
    advance();
}

/* =========================
   Utilities
   ========================= */

void Parser::advance() {
    current = lexer.nextToken();
}

bool Parser::match(TokenKind kind) {
    if (current.kind == kind) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(TokenKind kind, const char* msg) {
    if (current.kind != kind) {
        throw std::runtime_error(msg);
    }
    advance();
}

/* =========================
   Entry
   ========================= */

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (current.kind != TokenKind::End) {
        program->statements.push_back(parseDeclaration());
    }

    return program;
}

/* =========================
   Declarations
   ========================= */

std::unique_ptr<Declaration> Parser::parseDeclaration() {
    auto decl = std::make_unique<Declaration>();

    decl->type = parseType();

    // <a,b,c>
    do {
        if (current.kind != TokenKind::Identifier) {
            throw std::runtime_error("Expected identifier");
        }
        decl->names.push_back(current.lexeme);
        advance();
    } while (match(TokenKind::Comma));

    expect(TokenKind::Assign, "Expected '='");

    // 1,2,3
    do {
        decl->values.push_back(parseExpression());
    } while (match(TokenKind::Comma));

    return decl;
}

/* =========================
   Type
   ========================= */

TypeSpec Parser::parseType() {
    TypeSpec type;

    expect(TokenKind::LAngle, "Expected '<'");

    // scalar / vector inferred later
    type.kind = TypeKind::Scalar;

    expect(TokenKind::RAngle, "Expected '>'");

    return type;
}

/* =========================
   Expressions
   ========================= */

std::unique_ptr<Expr> Parser::parseExpression() {
    if (current.kind == TokenKind::Number) {
        auto lit = std::make_unique<Literal>();
        lit->value = std::stod(current.lexeme);
        advance();
        return lit;
    }

    throw std::runtime_error("Invalid expression");
}
