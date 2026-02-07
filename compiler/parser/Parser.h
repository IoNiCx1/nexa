#pragma once
#include "../lexer/Lexer.h"
#include "../ast/Ast.h"
#include <memory>

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<Program> parseProgram();

private:
    Lexer& lexer;
    Token current;

    /* core helpers */
    void advance();
    Token expect(TokenKind kind, const std::string& msg = "");

    /* statements */
    StmtPtr parseStatement();
    StmtPtr parseVarDecl();
    StmtPtr parseArrayDecl();
    StmtPtr parsePrintStatement();

    /* expressions (Pratt parser) */
    ExprPtr parseExpression(int minBp = 0);
    ExprPtr parsePrimary();

    /* operator helpers */
    int prefixBindingPower(TokenKind kind) const;
    int infixBindingPower(TokenKind kind) const;
    BinaryOp tokenToBinaryOp(TokenKind kind) const;
};
