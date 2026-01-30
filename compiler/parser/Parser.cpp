#include "Parser.h"
#include <stdexcept>
#include <vector>

Parser::Parser(Lexer& l) : lexer(l) { advance(); }

void Parser::advance() { current = lexer.nextToken(); }

Token Parser::expect(TokenKind kind, const std::string& msg) {
    if (current.kind != kind) {
        throw std::runtime_error(msg + " (Found '" + current.lexeme + "')");
    }
    Token t = current;
    advance();
    return t;
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (current.kind != TokenKind::EndOfFile) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

StmtPtr Parser::parseStatement() {
    if (current.kind == TokenKind::Print) {
        advance();
        auto expr = parseExpression();
        expect(TokenKind::Semicolon, "Expect ';' after print");
        return std::make_unique<PrintStmt>(std::move(expr));
    }
    if (current.kind == TokenKind::KeywordInt) {
        advance();
        auto name = expect(TokenKind::Identifier, "Expect name");
        expect(TokenKind::Assign, "Expect '='");
        auto expr = parseExpression();
        expect(TokenKind::Semicolon, "Expect ';'");
        return std::make_unique<VarDecl>(name.lexeme, TypeSpec(TypeKind::Int), std::move(expr));
    }

    if (current.kind == TokenKind::TYPE_I32_OPEN || 
        current.kind == TokenKind::TYPE_CHAR_OPEN || 
        current.kind == TokenKind::TYPE_STRING_OPEN) {
        return parseArrayDecl();
    }

    auto expr = parseExpression();
    // Path handling for accidental slashes
    while (current.kind == TokenKind::Slash) advance();
    
    expect(TokenKind::Semicolon, "Expect ';' after statement");
    return std::make_unique<PrintStmt>(std::move(expr)); 
}

StmtPtr Parser::parseArrayDecl() {
    TokenKind openKind = current.kind;
    TokenKind closeKind;
    TypeKind tKind;

    if (openKind == TokenKind::TYPE_I32_OPEN) {
        closeKind = TokenKind::TYPE_I32_CLOSE;
        tKind = TypeKind::IntArray;
    } else if (openKind == TokenKind::TYPE_CHAR_OPEN) {
        closeKind = TokenKind::TYPE_CHAR_CLOSE;
        tKind = TypeKind::CharArray;
    } else {
        closeKind = TokenKind::TYPE_STRING_CLOSE;
        tKind = TypeKind::StringArray;
    }

    advance(); 
    auto name = expect(TokenKind::Identifier, "Expect matrix name");
    expect(closeKind, "Expect closing bracket");

    if (current.kind == TokenKind::Semicolon) {
        advance();
        return std::make_unique<ArrayDecl>(name.lexeme, TypeSpec(tKind), std::vector<ExprPtr>{});
    }

    expect(TokenKind::Assign, "Expect '=' after matrix name");
    std::vector<ExprPtr> elems;
    do {
        elems.push_back(parseExpression());
    } while (current.kind == TokenKind::Comma && (advance(), true));

    expect(TokenKind::Semicolon, "Expect ';' at end of line");
    return std::make_unique<ArrayDecl>(name.lexeme, TypeSpec(tKind), std::move(elems));
}

ExprPtr Parser::parseExpression() {
    // 1. Handle Unary Minus (Negative Integers)
    if (current.kind == TokenKind::Minus) {
        advance();
        if (current.kind == TokenKind::IntegerLiteral) {
            int val = -std::stoi(current.lexeme);
            advance();
            return std::make_unique<IntegerLiteral>(val);
        }
        throw std::runtime_error("Expected number after '-'");
    }

    // 2. Handle Integer Literals
    if (current.kind == TokenKind::IntegerLiteral) {
        int val = std::stoi(current.lexeme);
        advance();
        return std::make_unique<IntegerLiteral>(val);
    }

    // 3. Handle String Literals
    if (current.kind == TokenKind::StringLiteral) {
        std::string val = current.lexeme;
        advance();
        return std::make_unique<StringLiteral>(val);
    }

    // 4. Handle Matrix References and Dot Products
    if (current.kind == TokenKind::TYPE_I32_OPEN || 
        current.kind == TokenKind::TYPE_CHAR_OPEN || 
        current.kind == TokenKind::TYPE_STRING_OPEN) {
        
        TokenKind openKind = current.kind;
        TokenKind closeKind = (openKind == TokenKind::TYPE_I32_OPEN) ? TokenKind::TYPE_I32_CLOSE :
                              (openKind == TokenKind::TYPE_CHAR_OPEN) ? TokenKind::TYPE_CHAR_CLOSE : 
                               TokenKind::TYPE_STRING_CLOSE;

        advance(); 
        auto name = expect(TokenKind::Identifier, "Expect name");
        expect(closeKind, "Expect closing bracket");

        if (current.kind == TokenKind::Dot) {
            advance();
            expect(openKind, "Mismatching matrix types in dot product");
            auto rhs = expect(TokenKind::Identifier, "Expect RHS name");
            expect(closeKind, "Expect closing bracket");
            return std::make_unique<DotExpr>(std::make_unique<VarRef>(name.lexeme), std::make_unique<VarRef>(rhs.lexeme));
        }
        return std::make_unique<VarRef>(name.lexeme);
    }

    // 5. Handle standard Variable References
    if (current.kind == TokenKind::Identifier) {
        std::string name = current.lexeme;
        advance();
        return std::make_unique<VarRef>(name);
    }
    
    // 6. Handle Bare Slashes (Paths)
    if (current.kind == TokenKind::Slash) {
        std::string s = current.lexeme;
        advance();
        return std::make_unique<StringLiteral>(s);
    }

    throw std::runtime_error("Unexpected token in expression: " + current.lexeme);
}
