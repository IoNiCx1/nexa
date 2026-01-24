#pragma once
#include "../lexer/Lexer.h"
#include "../ast/Ast.h"

class Parser {
public:
    explicit Parser(Lexer& lexer);

    std::unique_ptr<Program> parseProgram();

private:
    Lexer& lexer;
    Token current;

    void advance();
    StmtPtr parseStatement();
};
