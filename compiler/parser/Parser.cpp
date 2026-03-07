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

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd()) {
        program->statements.push_back(parseStatement());
    }
    return program;
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (match(TokenKind::Fn))
        return parseFunction();

    if (match(TokenKind::Return))
        return parseReturn();

    if (match(TokenKind::If))
        return parseIf();

    if (match(TokenKind::Print))
        return parsePrint();

    if (match(TokenKind::Loop))
        return parseLoop();

    // Variable Declaration: int x = 5;
    if (check(TokenKind::Int) || check(TokenKind::Double) ||
        check(TokenKind::String) || check(TokenKind::Bool)) {
        return parseVarDecl();
    }

    // Handle Expressions as Statements (Function calls or Assignments)
    auto expr = parseExpression();

    // 1. Assignment: x = 10;
    if (match(TokenKind::Assign)) {
        auto value = parseExpression();
        if (!match(TokenKind::Semicolon)) {
            std::cerr << "Expected ';' after assignment\n";
            exit(1);
        }
        return std::make_unique<AssignmentStmt>(std::move(expr), std::move(value));
    }

    // 2. Standalone Expression (like a function call): add(2, 3);
    if (!match(TokenKind::Semicolon)) {
        std::cerr << "Expected ';' after expression. Found: " << peek().lexeme << "\n";
        exit(1);
    }

    // We wrap the expression in a PrintStmt or a dedicated ExpressionStmt
    // If your AST doesn't have ExpressionStmt, PrintStmt works for debugging
    return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseFunction() {
    if (!check(TokenKind::Identifier)) {
        std::cerr << "Expected function name\n";
        exit(1);
    }

    std::string name = advance().lexeme;

    if (!match(TokenKind::LeftParen)) {
        std::cerr << "Expected '(' after function name\n";
        exit(1);
    }

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

    if (!match(TokenKind::RightParen)) {
        std::cerr << "Expected ')' after parameters\n";
        exit(1);
    }

    if (!match(TokenKind::Minus) || !match(TokenKind::Greater)) {
        std::cerr << "Expected '->' before return type\n";
        exit(1);
    }

    Type* returnType = parseType();

    if (!match(TokenKind::LeftBrace)) {
        std::cerr << "Expected '{' before function body\n";
        exit(1);
    }

    auto fn = std::make_unique<FunctionDecl>(name, returnType);
    for (auto& p : params) fn->params.push_back(p);

    while (!check(TokenKind::RightBrace) && !isAtEnd())
        fn->body.push_back(parseStatement());

    if (!match(TokenKind::RightBrace)) {
        std::cerr << "Expected '}' after function body\n";
        exit(1);
    }

    return fn;
}

std::unique_ptr<Stmt> Parser::parseReturn() {
    auto value = parseExpression();
    if (!match(TokenKind::Semicolon)) {
        std::cerr << "Expected ';' after return\n";
        exit(1);
    }
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::parseIf() {
    if (!match(TokenKind::LeftParen)) {
        std::cerr << "Expected '(' after if\n";
        exit(1);
    }
    auto condition = parseExpression();
    if (!match(TokenKind::RightParen)) {
        std::cerr << "Expected ')' after if condition\n";
        exit(1);
    }
    if (!match(TokenKind::LeftBrace)) {
        std::cerr << "Expected '{' after if\n";
        exit(1);
    }

    auto ifStmt = std::make_unique<IfStmt>(std::move(condition));
    while (!check(TokenKind::RightBrace))
        ifStmt->thenBranch.push_back(parseStatement());
    match(TokenKind::RightBrace);

    if (match(TokenKind::Else)) {
        if (!match(TokenKind::LeftBrace)) {
            std::cerr << "Expected '{' after else\n";
            exit(1);
        }
        while (!check(TokenKind::RightBrace))
            ifStmt->elseBranch.push_back(parseStatement());
        match(TokenKind::RightBrace);
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
    if (!match(TokenKind::Assign)) {
        std::cerr << "Expected '=' after variable name\n";
        exit(1);
    }
    auto initializer = parseExpression();
    if (!match(TokenKind::Semicolon)) {
        std::cerr << "Expected ';' after declaration\n";
        exit(1);
    }
    return std::make_unique<VarDeclStmt>(declaredType, name.lexeme, std::move(initializer));
}

Type* Parser::parseType() {
    Token t = advance();
    Type* base = nullptr;
    if (t.lexeme == "int") base = &TYPE_INT;
    else if (t.lexeme == "double") base = &TYPE_DOUBLE;
    else if (t.lexeme == "string") base = &TYPE_STRING;
    else if (t.lexeme == "bool") base = &TYPE_BOOL;
    else {
        std::cerr << "Unknown type: " << t.lexeme << "\n";
        exit(1);
    }

    if (match(TokenKind::LeftBracket)) {
        if (!match(TokenKind::RightBracket)) {
            std::cerr << "Expected ']' in array type\n";
            exit(1);
        }
        return new Type(TypeKind::Array, base);
    }
    return base;
}

std::unique_ptr<Stmt> Parser::parsePrint() {
    if (!match(TokenKind::LeftParen)) {
        std::cerr << "Expected '(' after print\n";
        exit(1);
    }
    auto expr = parseExpression();
    if (!match(TokenKind::RightParen)) {
        std::cerr << "Expected ')' after print\n";
        exit(1);
    }
    if (!match(TokenKind::Semicolon)) {
        std::cerr << "Expected ';' after print\n";
        exit(1);
    }
    return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseLoop() {
    if (!match(TokenKind::LeftParen)) {
        std::cerr << "Expected '(' after loop\n";
        exit(1);
    }
    if (!check(TokenKind::Identifier)) {
        std::cerr << "Expected iterator name\n";
        exit(1);
    }
    Token iterator = advance();
    if (!match(TokenKind::Comma)) {
        std::cerr << "Expected ',' in loop\n";
        exit(1);
    }
    auto count = parseExpression();
    if (!match(TokenKind::RightParen)) {
        std::cerr << "Expected ')' after loop\n";
        exit(1);
    }
    if (!match(TokenKind::LeftBrace)) {
        std::cerr << "Expected '{' after loop\n";
        exit(1);
    }

    auto loop = std::make_unique<LoopStmt>(iterator.lexeme, std::move(count));
    while (!check(TokenKind::RightBrace))
        loop->body.push_back(parseStatement());
    match(TokenKind::RightBrace);
    return loop;
}

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
        auto expr = std::make_unique<VariableExpr>(t.lexeme);
        // Important: Always check for postfix (calls/indexing) after an identifier
        return parsePostfix(std::move(expr));
    }

    if (t.kind == TokenKind::LeftParen) {
        auto expr = parseExpression();
        if (!match(TokenKind::RightParen)) {
             std::cerr << "Expected ')' after grouped expression\n";
             exit(1);
        }
        return expr;
    }

    if (t.kind == TokenKind::LeftBracket) {
        auto array = std::make_unique<ArrayLiteralExpr>();
        if (!check(TokenKind::RightBracket)) {
            do {
                array->elements.push_back(parseExpression());
            } while (match(TokenKind::Comma));
        }
        if (!match(TokenKind::RightBracket)) {
            std::cerr << "Expected ']' after array literal\n";
            exit(1);
        }
        return array;
    }

    std::cerr << "Unexpected expression token: " << t.lexeme << "\n";
    exit(1);
}

std::unique_ptr<Expr> Parser::parsePostfix(std::unique_ptr<Expr> expr) {
    while (true) {
        if (match(TokenKind::LeftParen)) {
            auto var = dynamic_cast<VariableExpr*>(expr.get());
            if (!var) {
                std::cerr << "Invalid function call target\n";
                exit(1);
            }
            auto call = std::make_unique<CallExpr>();
            call->callee = var->name;
            if (!check(TokenKind::RightParen)) {
                do {
                    call->arguments.push_back(parseExpression());
                } while (match(TokenKind::Comma));
            }
            if (!match(TokenKind::RightParen)) {
                std::cerr << "Expected ')' after arguments\n";
                exit(1);
            }
            expr = std::move(call);
        } else if (match(TokenKind::LeftBracket)) {
            auto index = parseExpression();
            if (!match(TokenKind::RightBracket)) {
                std::cerr << "Expected ']' after index\n";
                exit(1);
            }
            expr = std::make_unique<IndexExpr>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    return expr;
}