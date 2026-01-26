#include "Parser.h"
#include "../ast/Ast.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(Lexer& lexer) : lexer(lexer) {
    advance(); // load first token
}

void Parser::advance() {
    current = lexer.nextToken();
}

Token Parser::expect(TokenKind kind, const std::string& msg) {
    if (current.kind != kind) {
        throw std::runtime_error(
            msg.empty() ? 
            ("Unexpected token '" + current.lexeme + "' at line " + std::to_string(current.line) +
             ", column " + std::to_string(current.column))
            : msg
        );
    }
    Token tok = current;
    advance();
    return tok;
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (current.kind != TokenKind::EndOfFile) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

// Helper to handle your custom type openers like <int> or <<int>>
TypeSpec Parser::parseType() {
    if (current.kind == TokenKind::KeywordInt) {
        advance();
        return TypeSpec(TypeKind::Int, 1, 1);
    } 
    else if (current.kind == TokenKind::KeywordFloat) {
        advance();
        return TypeSpec(TypeKind::Float, 1, 1);
    }
    else if (current.kind == TokenKind::TYPE_I32_OPEN) { // Matches your '<'
        advance(); 
        expect(TokenKind::KeywordInt, "Expected 'int' inside < >");
        expect(TokenKind::TYPE_I32_CLOSE, "Expected '>' after type"); // Matches your '>'
        return TypeSpec(TypeKind::Int, 1, 1); 
    }
    else if (current.kind == TokenKind::TYPE_I64_OPEN) { // Matches your '<<'
        advance(); 
        expect(TokenKind::KeywordInt, "Expected 'int' inside << >>");
        expect(TokenKind::TYPE_I64_CLOSE, "Expected '>>' after type"); // Matches your '>>'
        return TypeSpec(TypeKind::Int, 1, 1);
    }
    throw std::runtime_error("Expected a type but found '" + current.lexeme + "'");
}

StmtPtr Parser::parseStatement() {
    // NEW: Check for print statement
    if (current.kind == TokenKind::Print) {
        return parsePrintStatement();
    }
    
    // Check for any token that can start a variable declaration
    if (current.kind == TokenKind::KeywordInt || 
        current.kind == TokenKind::KeywordFloat || 
        current.kind == TokenKind::TYPE_I32_OPEN || 
        current.kind == TokenKind::TYPE_I64_OPEN) {
        
        return parseVarDecl();
    }
    
    throw std::runtime_error(
        "Unknown statement starting with '" + current.lexeme +
        "' at line " + std::to_string(current.line) +
        ", column " + std::to_string(current.column)
    );
}

// NEW: Parse print statement
std::unique_ptr<PrintStmt> Parser::parsePrintStatement() {
    expect(TokenKind::Print, "Expected 'print'");
    
    // Parse the expression to print
    ExprPtr expr = parseExpression();
    
    expect(TokenKind::Semicolon, "Expected ';' after print statement");
    
    return std::make_unique<PrintStmt>(std::move(expr));
}

// NEW: Parse expressions (simple version for now)
ExprPtr Parser::parseExpression() {
    // Handle different literal types
    if (current.kind == TokenKind::IntegerLiteral) {
        Token tok = current;
        advance();
        int value = std::stoi(tok.lexeme);
        return std::make_unique<IntegerLiteral>(value);
    }
    else if (current.kind == TokenKind::FloatLiteral) {
        Token tok = current;
        advance();
        float value = std::stof(tok.lexeme);
        return std::make_unique<FloatLiteral>(value);
    }
    else if (current.kind == TokenKind::StringLiteral) {
        Token tok = current;
        advance();
        return std::make_unique<StringLiteral>(tok.lexeme);
    }
    else if (current.kind == TokenKind::CharLiteral) {
        Token tok = current;
        advance();
        char value = tok.lexeme.empty() ? '\0' : tok.lexeme[0];
        return std::make_unique<CharLiteral>(value);
    }
    else if (current.kind == TokenKind::BoolLiteral) {
        Token tok = current;
        advance();
        bool value = (tok.lexeme == "true");
        return std::make_unique<BoolLiteral>(value);
    }
    else if (current.kind == TokenKind::Identifier) {
        Token tok = current;
        advance();
        return std::make_unique<VarRef>(tok.lexeme);
    }
    
    throw std::runtime_error(
        "Expected expression but found '" + current.lexeme +
        "' at line " + std::to_string(current.line)
    );
}

std::unique_ptr<VarDecl> Parser::parseVarDecl() {
    TypeSpec type = parseType();
    Token nameTok = expect(TokenKind::Identifier, "Expected variable name");
    std::string name = nameTok.lexeme;
    
    expect(TokenKind::Assign, "Expected '='");
    
    // Use parseExpression instead of hardcoded integer literal
    ExprPtr init = parseExpression();
    
    // Be very explicit here
    if (current.kind != TokenKind::Semicolon) {
        throw std::runtime_error("Expected ';' but found '" + current.lexeme + 
                                 "' at line " + std::to_string(current.line));
    }
    advance(); // Consume the semicolon
    
    return std::make_unique<VarDecl>(name, type, std::move(init));
}

