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

    void advance();
    bool match(TokenKind kind);
    void expect(TokenKind kind, const char* msg);

    std::unique_ptr<Declaration> parseDeclaration();
    TypeSpec parseType();
    std::unique_ptr<Expr> parseExpression();
};
