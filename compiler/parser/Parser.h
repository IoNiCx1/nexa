#pragma once
#include "../lexer/Lexer.h"
#include "../lexer/Token.h"
#include "../ast/Ast.h"
#include <memory>

class Parser {
public:
    explicit Parser(Lexer& lexer);
    std::unique_ptr<Program> parseProgram();

private:
    Lexer& lexer;
    Token current;

    void advance();
    Token expect(TokenKind kind, const std::string& msg = "");

    StmtPtr parseStatement();
    StmtPtr parseLoop();
    StmtPtr parseArrayDecl();

    ExprPtr parseExpression();
};
