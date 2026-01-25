#pragma once
#include "../lexer/Lexer.h"
#include "../ast/Ast.h"
#include <memory>
#include <string>

class Parser {
public:
    explicit Parser(Lexer& lexer);

    // Parses the whole program
    std::unique_ptr<Program> parseProgram();

private:
    Lexer& lexer;
    Token current;

    // Advance to the next token
    void advance();

    // Expect a token of a specific kind, and return it
    Token expect(TokenKind kind, const std::string& msg = "");

    // Parse a single statement
    StmtPtr parseStatement();
    
    // Helper to handle types like int, <int>, etc.
    TypeSpec parseType();
    
    // Fixed: changed ValDecl to VarDecl
    std::unique_ptr<VarDecl> parseVarDecl(); 
};
