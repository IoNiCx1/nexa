#ifndef NEXA_PARSER_H
#define NEXA_PARSER_H

#include "../ast/Ast.h"
#include "../lexer/Token.h"
#include "../sema/Type.h"

#include <vector>
#include <memory>

namespace nexa {

class Parser {
public:
    Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> parseProgram();

private:
    const std::vector<Token>& tokens;
    size_t current = 0;

    // =============================
    // Core Helpers
    // =============================
    Token peek();
    Token previous();
    bool isAtEnd();
    Token advance();
    bool check(TokenKind kind);
    bool match(TokenKind kind);

    // =============================
    // Statements
    // =============================
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseAssignment();
    std::unique_ptr<Stmt> parsePrint();
    std::unique_ptr<Stmt> parseLoop();

    // =============================
    // Expressions (Pratt)
    // =============================
    std::unique_ptr<Expr> parseExpression(int precedence = 0);
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parsePostfix(std::unique_ptr<Expr> expr);

    int getPrecedence(TokenKind kind);

    // =============================
    // Utilities
    // =============================
    Type* parseType();
};

}

#endif