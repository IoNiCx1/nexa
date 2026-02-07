#include "Parser.h"
#include <stdexcept>

/* ================= SETUP ================= */

Parser::Parser(Lexer& l) : lexer(l) {
    advance();
}

void Parser::advance() {
    current = lexer.nextToken();
}

Token Parser::expect(TokenKind kind, const std::string& msg) {
    if (current.kind != kind) {
        throw std::runtime_error(msg.empty()
            ? "Unexpected token: " + current.lexeme
            : msg);
    }
    Token t = current;
    advance();
    return t;
}

/* ================= PROGRAM ================= */

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (current.kind != TokenKind::EndOfFile) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

/* ================= STATEMENTS ================= */

StmtPtr Parser::parseStatement() {
    if (current.kind == TokenKind::KeywordInt) {
        return parseVarDecl();
    }

    if (current.kind == TokenKind::Print) {
        return parsePrintStatement();
    }

    if (current.kind == TokenKind::TYPE_I32_OPEN ||
        current.kind == TokenKind::TYPE_CHAR_OPEN ||
        current.kind == TokenKind::TYPE_STRING_OPEN) {
        return parseArrayDecl();
    }

    throw std::runtime_error("Invalid statement");
}

StmtPtr Parser::parseVarDecl() {
    expect(TokenKind::KeywordInt);
    Token name = expect(TokenKind::Identifier, "Expected variable name");
    expect(TokenKind::Assign, "Expected '='");

    ExprPtr init = parseExpression();
    expect(TokenKind::Semicolon, "Expected ';'");

    return std::make_unique<VarDecl>(
        name.lexeme,
        TypeSpec(TypeKind::Int),
        std::move(init)
    );
}

StmtPtr Parser::parsePrintStatement() {
    expect(TokenKind::Print);
    ExprPtr expr = parseExpression();
    expect(TokenKind::Semicolon, "Expected ';'");
    return std::make_unique<PrintStmt>(std::move(expr));
}

/* ================= ARRAY DECL ================= */

StmtPtr Parser::parseArrayDecl() {
    TokenKind open = current.kind;
    TypeKind tkind;
    TokenKind close;

    if (open == TokenKind::TYPE_I32_OPEN) {
        tkind = TypeKind::IntArray;
        close = TokenKind::TYPE_I32_CLOSE;
    } else if (open == TokenKind::TYPE_CHAR_OPEN) {
        tkind = TypeKind::CharArray;
        close = TokenKind::TYPE_CHAR_CLOSE;
    } else {
        tkind = TypeKind::StringArray;
        close = TokenKind::TYPE_STRING_CLOSE;
    }

    advance();
    Token name = expect(TokenKind::Identifier, "Expected array name");
    expect(close, "Expected closing bracket");

    std::vector<ExprPtr> elems;

    if (current.kind == TokenKind::Assign) {
        advance();
        do {
            elems.push_back(parseExpression());
            if (current.kind != TokenKind::Comma) break;
            advance();
        } while (true);
    }

    expect(TokenKind::Semicolon, "Expected ';'");

    return std::make_unique<ArrayDecl>(
        name.lexeme,
        TypeSpec(tkind),
        std::move(elems)
    );
}

/* ================= PRATT PARSER ================= */

ExprPtr Parser::parseExpression(int minBp) {
    ExprPtr lhs;

    /* prefix */
    if (current.kind == TokenKind::Minus) {
        advance();
        int bp = prefixBindingPower(TokenKind::Minus);
        ExprPtr rhs = parseExpression(bp);
        lhs = std::make_unique<UnaryExpr>(UnaryOp::Negate, std::move(rhs));
    } else {
        lhs = parsePrimary();
    }

    /* infix loop */
    while (true) {
        int bp = infixBindingPower(current.kind);
        if (bp < minBp) break;

        TokenKind opTok = current.kind;
        advance();

        ExprPtr rhs = parseExpression(bp + 1);
        lhs = std::make_unique<BinaryExpr>(
            tokenToBinaryOp(opTok),
            std::move(lhs),
            std::move(rhs)
        );
    }

    return lhs;
}

/* ================= PRIMARY ================= */

ExprPtr Parser::parsePrimary() {
    if (current.kind == TokenKind::IntegerLiteral) {
        int val = std::stoi(current.lexeme);
        advance();
        return std::make_unique<IntegerLiteral>(val);
    }

    if (current.kind == TokenKind::Identifier) {
        std::string name = current.lexeme;
        advance();
        return std::make_unique<VarRef>(name);
    }

    if (current.kind == TokenKind::StringLiteral) {
        std::string s = current.lexeme;
        advance();
        return std::make_unique<StringLiteral>(s);
    }

    if (current.kind == TokenKind::TYPE_CHAR_OPEN) {
        advance();
        ExprPtr expr = parseExpression();
        expect(TokenKind::TYPE_CHAR_CLOSE, "Expected ')'");
        return expr;
    }

    throw std::runtime_error("Invalid expression");
}

/* ================= BINDING POWERS ================= */

int Parser::prefixBindingPower(TokenKind kind) const {
    if (kind == TokenKind::Minus) return 7;
    return 0;
}

int Parser::infixBindingPower(TokenKind kind) const {
    switch (kind) {
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent:
            return 5;
        case TokenKind::Plus:
        case TokenKind::Minus:
            return 3;
        default:
            return -1;
    }
}

BinaryOp Parser::tokenToBinaryOp(TokenKind kind) const {
    switch (kind) {
        case TokenKind::Plus:    return BinaryOp::Add;
        case TokenKind::Minus:   return BinaryOp::Sub;
        case TokenKind::Star:    return BinaryOp::Mul;
        case TokenKind::Slash:   return BinaryOp::Div;
        case TokenKind::Percent:return BinaryOp::Mod;
        default:
            throw std::runtime_error("Invalid binary operator");
    }
}
