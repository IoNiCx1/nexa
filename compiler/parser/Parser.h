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

    // ── Core helpers ──────────────────────────
    Token peek();
    Token previous();
    bool  isAtEnd();
    Token advance();
    bool  check(TokenKind kind);
    bool  match(TokenKind kind);
    Token consume(TokenKind kind);

    // ── Statements ────────────────────────────
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseIf();
    std::unique_ptr<Stmt> parseVarDecl();
    std::unique_ptr<Stmt> parseAssignment();
    std::unique_ptr<Stmt> parsePrint();
    std::unique_ptr<Stmt> parseLoop();
    std::unique_ptr<Stmt> parseFunction();
    std::unique_ptr<Stmt> parseReturn();
    std::unique_ptr<Stmt> parseStructDecl();
    std::unique_ptr<Stmt> parseConstructorStmt();

    // ── File handling ─────────────────────────
    std::unique_ptr<Stmt>     parseImp();
    std::unique_ptr<Stmt>     parseFileStmt();
    std::unique_ptr<FileExpr> parseFileExpr();

    // ── Expressions ───────────────────────────
    std::unique_ptr<Expr> parseExpression(int precedence = 0);
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parsePostfix(std::unique_ptr<Expr> expr);
    std::unique_ptr<Expr> parseTensorLiteral();
    int getPrecedence(TokenKind kind);

    // ── Utilities ─────────────────────────────
    Type*       parseType();
    const char* tokenKindName(TokenKind k);
};

} // namespace nexa

#endif
