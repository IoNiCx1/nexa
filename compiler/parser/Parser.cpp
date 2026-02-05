#include "Parser.h"
#include <stdexcept>

/* ================= CONSTRUCTOR ================= */

Parser::Parser(Lexer& l) : lexer(l) { advance(); }

void Parser::advance() { current = lexer.nextToken(); }

Token Parser::expect(TokenKind kind, const std::string& msg) {
    if (current.kind != kind)
        throw std::runtime_error(msg + " (found '" + current.lexeme + "')");
    Token t = current;
    advance();
    return t;
}

/* ================= PROGRAM ================= */

std::unique_ptr<Program> Parser::parseProgram() {
    auto p = std::make_unique<Program>();
    while (current.kind != TokenKind::EndOfFile)
        p->statements.push_back(parseStatement());
    return p;
}

/* ================= STATEMENTS ================= */

StmtPtr Parser::parseStatement() {

    if (current.kind == TokenKind::Print) {
        advance();
        auto e = parseExpression();
        expect(TokenKind::Semicolon, "Expect ';'");
        return std::make_unique<PrintStmt>(std::move(e));
    }

    if (current.kind == TokenKind::KeywordInt) {
        advance();
        auto name = expect(TokenKind::Identifier, "Expect name");
        expect(TokenKind::Assign, "Expect '='");
        auto e = parseExpression();
        expect(TokenKind::Semicolon, "Expect ';'");
        return std::make_unique<VarDecl>(
            name.lexeme, TypeSpec(TypeKind::Int), std::move(e));
    }

    if (current.kind == TokenKind::TYPE_I32_OPEN ||
        current.kind == TokenKind::TYPE_STRING_OPEN)
        return parseArrayDecl();

    auto e = parseExpression();
    expect(TokenKind::Semicolon, "Expect ';'");
    return std::make_unique<ExprStmt>(std::move(e));
}

/* ================= ARRAY / STRING DECL ================= */

StmtPtr Parser::parseArrayDecl() {

    TypeKind type;
    TokenKind close;

    if (current.kind == TokenKind::TYPE_I32_OPEN) {
        type = TypeKind::IntArray;
        close = TokenKind::TYPE_I32_CLOSE;
    } else {
        type = TypeKind::String;
        close = TokenKind::TYPE_STRING_CLOSE;
    }

    advance();
    auto name = expect(TokenKind::Identifier, "Expect name");
    expect(close, "Expect closing");

    expect(TokenKind::Assign, "Expect '='");

    if (type == TypeKind::String) {
        auto v = parseExpression();
        expect(TokenKind::Semicolon, "Expect ';'");
        return std::make_unique<VarDecl>(
            name.lexeme, TypeSpec(TypeKind::String), std::move(v));
    }

    std::vector<ExprPtr> elems;
    do {
        elems.push_back(parseExpression());
        if (current.kind == TokenKind::Comma) advance();
        else break;
    } while (true);

    expect(TokenKind::Semicolon, "Expect ';'");
    return std::make_unique<ArrayDecl>(
        name.lexeme, TypeSpec(TypeKind::IntArray), std::move(elems));
}

/* ================= EXPRESSIONS ================= */

ExprPtr Parser::parseExpression() {

    /* unary minus */
    if (current.kind == TokenKind::Minus) {
        advance();
        auto val = expect(
            TokenKind::IntegerLiteral,
            "Expected number after '-'");
        int v = -std::stoi(val.lexeme);
        return std::make_unique<IntegerLiteral>(v);
    }

    if (current.kind == TokenKind::IntegerLiteral) {
        int v = std::stoi(current.lexeme);
        advance();
        return std::make_unique<IntegerLiteral>(v);
    }

    if (current.kind == TokenKind::StringLiteral) {
        std::string v = current.lexeme;
        advance();
        return std::make_unique<StringLiteral>(v);
    }

    if (current.kind == TokenKind::TYPE_I32_OPEN ||
        current.kind == TokenKind::TYPE_STRING_OPEN) {

        TypeKind type =
            current.kind == TokenKind::TYPE_I32_OPEN
                ? TypeKind::IntArray
                : TypeKind::String;

        TokenKind close =
            current.kind == TokenKind::TYPE_I32_OPEN
                ? TokenKind::TYPE_I32_CLOSE
                : TokenKind::TYPE_STRING_CLOSE;

        advance();
        auto name = expect(TokenKind::Identifier, "Expect name");
        expect(close, "Expect closing");

        return std::make_unique<TypedRefExpr>(name.lexeme, type);
    }

    if (current.kind == TokenKind::Identifier) {
        std::string n = current.lexeme;
        advance();
        return std::make_unique<VarRef>(n);
    }

    throw std::runtime_error("Invalid expression");
}
