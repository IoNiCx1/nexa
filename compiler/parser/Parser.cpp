#include "Parser.h"
#include <stdexcept>

/* =========================
   Utility
   ========================= */

Token Parser::peek() const {
    return tokens[current];
}

Token Parser::advance() {
    return tokens[current++];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

Token Parser::expect(TokenType type) {
    if (peek().type != type) {
        throw std::runtime_error(
            "Expected '" + tokenTypeToString(type) + "'");
    }
    return advance();
}

/* =========================
   Entry
   ========================= */

Program Parser::parse() {
    Program program;

    while (peek().type != TokenType::EOFToken) {
        parseDeclaration(program);
    }

    return program;
}

/* =========================
   Declarations
   ========================= */

void Parser::parseDeclaration(Program& program) {
    TypeSpec type = parseTypeSpec();

    std::vector<std::string> names = parseIdentifierList();

    expect(TokenType::Equal);

    std::vector<std::unique_ptr<Expr>> values;
    do {
        values.push_back(parseExpression());
    } while (match(TokenType::Comma));

    if (names.size() != values.size()) {
        throw std::runtime_error(
            "Variable count does not match value count");
    }

    auto decl = std::make_unique<Declaration>();
    decl->type = type;
    decl->names = std::move(names);
    decl->values = std::move(values);

    program.statements.push_back(std::move(decl));
}

/* =========================
   Type Spec
   ========================= */

TypeSpec Parser::parseTypeSpec() {
    expect(TokenType::Less);

    TypeSpec spec;
    spec.kind = TypeKind::Int; // default scalar
    spec.rows = 1;
    spec.cols = 1;

    return spec;
}

/* =========================
   Identifier List
   ========================= */

std::vector<std::string> Parser::parseIdentifierList() {
    std::vector<std::string> names;

    do {
        Token id = expect(TokenType::Identifier);
        names.push_back(id.lexeme);
    } while (match(TokenType::Comma));

    expect(TokenType::Greater);
    return names;
}

/* =========================
   Expressions
   ========================= */

std::unique_ptr<Expr> Parser::parseExpression() {
    Token tok = advance();

    if (tok.type == TokenType::Number) {
        auto lit = std::make_unique<Literal>();
        lit->value = std::stod(tok.lexeme);
        return lit;
    }

    throw std::runtime_error("Invalid expression");
}
