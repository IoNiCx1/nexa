#include "Parser.h"
#include "../ast/Ast.h"

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    advance();
}

void Parser::advance() {
    current = lexer.nextToken();
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();

    while (current.kind != TokenKind::EndOfFile) {
        program->statements.push_back(parseStatement());
    }

    return program;
}

StmtPtr Parser::parseStatement() {
    // Placeholder: int x = 5;
    if (current.kind == TokenKind::KeywordInt) {
        advance(); // consume 'int'

        std::string name = current.lexeme;
        advance(); // identifier

        advance(); // '='

        int value = std::stoi(current.lexeme);
        advance(); // number

        advance(); // ';'

        TypeSpec type{TypeKind::Int, 0, 0};
        auto init = std::make_unique<IntegerLiteral>(value);

        return std::make_unique<VarDecl>(name, type, std::move(init));
    }

    // fallback
    advance();
    return nullptr;
}
