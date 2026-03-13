#include "Parser.h"
#include <iostream>

using namespace nexa;

Parser::Parser(const std::vector<Token>& toks)
    : tokens(toks), current(0) {}

Token Parser::peek() { return tokens[current]; }
Token Parser::previous() { return tokens[current - 1]; }
bool Parser::isAtEnd() { return peek().kind == TokenKind::END; }

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenKind kind) {
    if (isAtEnd()) return false;
    return peek().kind == kind;
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenKind kind) {
    if (check(kind))
        return advance();

    std::cerr << "Parser error: expected token but got "
              << peek().lexeme << " at index " << current << std::endl;
    exit(1);
}

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

// =============================
// Statement Parsing
// =============================

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (match(TokenKind::Fn)) return parseFunction();
    if (match(TokenKind::Return)) return parseReturn();
    if (match(TokenKind::If)) return parseIf();
    if (match(TokenKind::Print)) return parsePrint();
    if (match(TokenKind::Loop)) return parseLoop();

    if (check(TokenKind::Int) || check(TokenKind::Double) ||
        check(TokenKind::String) || check(TokenKind::Bool) ||
        check(TokenKind::Tensor)) {
        return parseVarDecl();
    }

    auto expr = parseExpression();

    // Handle Assignments (x = value)
    if (match(TokenKind::Assign)) {
        auto value = parseExpression();
        consume(TokenKind::Semicolon);
        return std::make_unique<AssignmentStmt>(std::move(expr), std::move(value));
    }

    consume(TokenKind::Semicolon);
    // Treat standalone expressions as print calls (repl behavior) or just statements
    return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseFunction() {
    if (!check(TokenKind::Identifier)) {
        std::cerr << "Expected function name\n";
        exit(1);
    }
    std::string name = advance().lexeme;

    consume(TokenKind::LeftParen);
    std::vector<std::pair<std::string, Type*>> params;

    if (!check(TokenKind::RightParen)) {
        do {
            Type* paramType = parseType();
            if (!check(TokenKind::Identifier)) {
                std::cerr << "Expected parameter name\n";
                exit(1);
            }
            std::string paramName = advance().lexeme;
            params.push_back({paramName, paramType});
        } while (match(TokenKind::Comma));
    }

    consume(TokenKind::RightParen);
    consume(TokenKind::Minus);
    consume(TokenKind::Greater);

    Type* returnType = parseType();
    consume(TokenKind::LeftBrace);

    auto fn = std::make_unique<FunctionDecl>(name, returnType);
    for (auto& p : params)
        fn->params.push_back(p);

    while (!check(TokenKind::RightBrace) && !isAtEnd())
        fn->body.push_back(parseStatement());

    consume(TokenKind::RightBrace);
    return fn;
}

std::unique_ptr<Stmt> Parser::parseReturn() {
    auto value = parseExpression();
    consume(TokenKind::Semicolon);
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::parseIf() {
    consume(TokenKind::LeftParen);
    auto condition = parseExpression();
    consume(TokenKind::RightParen);
    consume(TokenKind::LeftBrace);

    auto ifStmt = std::make_unique<IfStmt>(std::move(condition));
    while (!check(TokenKind::RightBrace))
        ifStmt->thenBranch.push_back(parseStatement());

    consume(TokenKind::RightBrace);
    if (match(TokenKind::Else)) {
        consume(TokenKind::LeftBrace);
        while (!check(TokenKind::RightBrace))
            ifStmt->elseBranch.push_back(parseStatement());
        consume(TokenKind::RightBrace);
    }
    return ifStmt;
}

std::unique_ptr<Stmt> Parser::parseVarDecl() {
    Type* declaredType = parseType();
    if (!check(TokenKind::Identifier)) {
        std::cerr << "Expected variable name\n";
        exit(1);
    }
    Token name = advance();

    consume(TokenKind::Assign);
    auto initializer = parseExpression();
    consume(TokenKind::Semicolon);

    return std::make_unique<VarDeclStmt>(declaredType, name.lexeme, std::move(initializer));
}

Type* Parser::parseType() {
    Token t = advance();
    Type* base = nullptr;

    if (t.lexeme == "int") base = &TYPE_INT;
    else if (t.lexeme == "double") base = &TYPE_DOUBLE;
    else if (t.lexeme == "string") base = &TYPE_STRING;
    else if (t.lexeme == "bool") base = &TYPE_BOOL;
    else if (t.lexeme == "tensor") base = &TYPE_TENSOR;
    else {
        std::cerr << "Unknown type: " << t.lexeme << "\n";
        exit(1);
    }

    if (match(TokenKind::LeftBracket)) {
        consume(TokenKind::RightBracket);
        return new Type(TypeKind::Array, base);
    }
    return base;
}

std::unique_ptr<Stmt> Parser::parsePrint() {
    consume(TokenKind::LeftParen);
    auto expr = parseExpression();
    consume(TokenKind::RightParen);
    consume(TokenKind::Semicolon);
    return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseLoop() {
    consume(TokenKind::LeftParen);
    if (!check(TokenKind::Identifier)) {
        std::cerr << "Expected iterator name\n";
        exit(1);
    }
    Token iterator = advance();

    consume(TokenKind::Comma);
    auto count = parseExpression();
    consume(TokenKind::RightParen);
    consume(TokenKind::LeftBrace);

    auto loop = std::make_unique<LoopStmt>(iterator.lexeme, std::move(count));
    while (!check(TokenKind::RightBrace))
        loop->body.push_back(parseStatement());

    consume(TokenKind::RightBrace);
    return loop;
}

// =============================
// Expression Parsing
// =============================

int Parser::getPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind::Star:
        case TokenKind::Slash: return 3;
        case TokenKind::Plus:
        case TokenKind::Minus: return 2;
        case TokenKind::Less:
        case TokenKind::LessEqual:
        case TokenKind::Greater:
        case TokenKind::GreaterEqual:
        case TokenKind::EqualEqual:
        case TokenKind::NotEqual: return 1;
        default: return 0;
    }
}

std::unique_ptr<Expr> Parser::parseExpression(int precedence) {
    auto left = parsePrimary();
    
    // Handle postfix operators like array indexing: arr[i]
    while (match(TokenKind::LeftBracket)) {
        auto index = parseExpression();
        consume(TokenKind::RightBracket);
        left = std::make_unique<IndexExpr>(std::move(left), std::move(index));
    }

    while (!isAtEnd() && getPrecedence(peek().kind) > precedence) {
        Token op = advance();
        int newPrec = getPrecedence(op.kind);
        auto right = parseExpression(newPrec);
        left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (isAtEnd()) {
        std::cerr << "Unexpected end of file while parsing expression\n";
        exit(1);
    }

    Token t = advance();

    if (t.kind == TokenKind::IntegerLiteral)
        return std::make_unique<IntegerLiteral>(std::stoi(t.lexeme));
    if (t.kind == TokenKind::FloatLiteral)
        return std::make_unique<DoubleLiteral>(std::stod(t.lexeme));
    if (t.kind == TokenKind::StringLiteral)
        return std::make_unique<StringLiteral>(t.lexeme);
    if (t.kind == TokenKind::True)
        return std::make_unique<BoolLiteral>(true);
    if (t.kind == TokenKind::False)
        return std::make_unique<BoolLiteral>(false);
    
    if (t.kind == TokenKind::Identifier) {
        return std::make_unique<VariableExpr>(t.lexeme);
    }

    // Parentheses: ( expr ) -> GroupingExpr (Critical for math and Semantic Analysis)
    if (t.kind == TokenKind::LeftParen) {
        auto expr = parseExpression();
        consume(TokenKind::RightParen);
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    // Tensor literal [[...]]
    if (t.kind == TokenKind::LeftBracket && check(TokenKind::LeftBracket)) {
        current--; // Back up to let parseTensorLiteral handle the outer bracket
        return parseTensorLiteral();
    }

    // Array literal [...]
    if (t.kind == TokenKind::LeftBracket) {
        auto array = std::make_unique<ArrayLiteralExpr>();
        if (!check(TokenKind::RightBracket)) {
            do {
                array->elements.push_back(parseExpression());
            } while (match(TokenKind::Comma));
        }
        consume(TokenKind::RightBracket);
        return array;
    }

    std::cerr << "Unexpected expression token: " << t.lexeme << "\n";
    exit(1);
}

std::unique_ptr<Expr> Parser::parseTensorLiteral() {
    std::vector<std::vector<std::unique_ptr<Expr>>> rows;

    consume(TokenKind::LeftBracket); // Outer [

    while (!check(TokenKind::RightBracket)) {
        consume(TokenKind::LeftBracket); // Inner [

        std::vector<std::unique_ptr<Expr>> row;
        if (!check(TokenKind::RightBracket)) {
            do {
                row.push_back(parseExpression());
            } while (match(TokenKind::Comma));
        }

        consume(TokenKind::RightBracket); // Inner ]
        rows.push_back(std::move(row));

        if (!match(TokenKind::Comma))
            break;
    }

    consume(TokenKind::RightBracket); // Outer ]
    return std::make_unique<TensorLiteralExpr>(std::move(rows));
}