#include "Parser.h"
#include <iostream>

using namespace nexa;

Parser::Parser(const std::vector<Token>& toks)
    : tokens(toks), current(0) {}

Token Parser::peek()     { return tokens[current]; }
Token Parser::previous() { return tokens[current - 1]; }
bool  Parser::isAtEnd()  { return peek().kind == TokenKind::END; }

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenKind kind) {
    if (isAtEnd()) return false;
    return peek().kind == kind;
}

bool Parser::match(TokenKind kind) {
    if (check(kind)) { advance(); return true; }
    return false;
}

Token Parser::consume(TokenKind kind) {
    if (check(kind)) return advance();
    std::cerr << "[parser] error: expected '" << tokenKindName(kind)
              << "' but got '" << peek().lexeme
              << "' at line " << peek().line
              << " col "      << peek().column << "\n";
    exit(1);
}

// Human-readable name for a TokenKind — used in error messages only
const char* Parser::tokenKindName(TokenKind k) {
    switch (k) {
        case TokenKind::LeftParen:    return "(";
        case TokenKind::RightParen:   return ")";
        case TokenKind::LeftBrace:    return "{";
        case TokenKind::RightBrace:   return "}";
        case TokenKind::LeftBracket:  return "[";
        case TokenKind::RightBracket: return "]";
        case TokenKind::Semicolon:    return ";";
        case TokenKind::Comma:        return ",";
        case TokenKind::Assign:       return "=";
        case TokenKind::Minus:        return "-";
        case TokenKind::Greater:      return ">";
        default:                      return "<token>";
    }
}

// =============================
// Program
// =============================

std::unique_ptr<Program> Parser::parseProgram() {
    auto program = std::make_unique<Program>();
    while (!isAtEnd())
        program->statements.push_back(parseStatement());
    return program;
}

// =============================
// Statements
// =============================

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (match(TokenKind::Struct)) return parseStructDecl();
    if (match(TokenKind::Fn))     return parseFunction();
    if (match(TokenKind::Return)) return parseReturn();
    if (match(TokenKind::If))     return parseIf();
    if (match(TokenKind::Print))  return parsePrint();
    if (match(TokenKind::Loop))   return parseLoop();

    if (check(TokenKind::Int)    || check(TokenKind::Double) ||
        check(TokenKind::String) || check(TokenKind::Bool)   ||
        check(TokenKind::Tensor) || check(TokenKind::Struct))
        return parseVarDecl();

    if (check(TokenKind::Identifier) && current + 1 < tokens.size() && 
        tokens[current + 1].kind == TokenKind::Identifier)
        return parseVarDecl();

    // Expression statement — could be an assignment or a standalone call
    auto expr = parseExpression();

    if (match(TokenKind::Assign)) {
        auto value = parseExpression();
        consume(TokenKind::Semicolon);
        return std::make_unique<AssignmentStmt>(std::move(expr), std::move(value));
    }

    consume(TokenKind::Semicolon);
    return std::make_unique<PrintStmt>(std::move(expr));
}

// -- struct name { type field; ... } --
std::unique_ptr<Stmt> Parser::parseStructDecl()
{
    if (!check(TokenKind::Identifier)) 
    {
        std::cerr << "[parser] error: expected struct name\n"; exit(1);
    }
    std::string name = advance().lexeme;
    consume(TokenKind::LeftBrace);

    auto decl = std::make_unique<StructDecl>(name);

    while (!check(TokenKind::RightBrace) && !isAtEnd())
    {
        Type* fieldtype = parseType();
        if (!check(TokenKind::Identifier))
        {
            std::cerr << "[parser] error: expected field name\n"; exit(1);
        }
        std::string fieldName = advance().lexeme;
        consume(TokenKind::Semicolon);
        decl->fields.push_back({fieldName, fieldtype});
    }

    consume(TokenKind::RightBrace);
    return decl;
}

// ── fn name(type param, ...) -> type { body } ──
std::unique_ptr<Stmt> Parser::parseFunction() {
    if (!check(TokenKind::Identifier)) {
        std::cerr << "[parser] error: expected function name\n"; exit(1);
    }
    std::string name = advance().lexeme;

    consume(TokenKind::LeftParen);

    std::vector<std::pair<std::string, Type*>> params;
    if (!check(TokenKind::RightParen)) {
        do {
            Type* paramType = parseType();
            if (!check(TokenKind::Identifier)) {
                std::cerr << "[parser] error: expected parameter name\n"; exit(1);
            }
            std::string paramName = advance().lexeme;
            params.push_back({paramName, paramType});
        } while (match(TokenKind::Comma));
    }

    consume(TokenKind::RightParen);
    consume(TokenKind::Arrow);
    Type* returnType = parseType();
    consume(TokenKind::LeftBrace);

    auto fn = std::make_unique<FunctionDecl>(name, returnType);
    fn->params = std::move(params);

    while (!check(TokenKind::RightBrace) && !isAtEnd())
        fn->body.push_back(parseStatement());

    consume(TokenKind::RightBrace);
    return fn;
}

// ── return expr; ──
std::unique_ptr<Stmt> Parser::parseReturn() {
    // Allow bare `return;` for void functions
    if (check(TokenKind::Semicolon)) {
        advance();
        return std::make_unique<ReturnStmt>(nullptr);
    }
    auto value = parseExpression();
    consume(TokenKind::Semicolon);
    return std::make_unique<ReturnStmt>(std::move(value));
}

// ── if (cond) { ... } else { ... } ──
std::unique_ptr<Stmt> Parser::parseIf() {
    consume(TokenKind::LeftParen);
    auto condition = parseExpression();
    consume(TokenKind::RightParen);
    consume(TokenKind::LeftBrace);

    auto ifStmt = std::make_unique<IfStmt>(std::move(condition));
    while (!check(TokenKind::RightBrace) && !isAtEnd())
        ifStmt->thenBranch.push_back(parseStatement());
    consume(TokenKind::RightBrace);

    if (match(TokenKind::Else)) {
        consume(TokenKind::LeftBrace);
        while (!check(TokenKind::RightBrace) && !isAtEnd())
            ifStmt->elseBranch.push_back(parseStatement());
        consume(TokenKind::RightBrace);
    }
    return ifStmt;
}

// ── type name = expr; ──
std::unique_ptr<Stmt> Parser::parseVarDecl() {
    Type* declaredType = parseType();
    if (!check(TokenKind::Identifier)) {
        std::cerr << "[parser] error: expected variable name\n"; exit(1);
    }
    std::string name = advance().lexeme;
    consume(TokenKind::Assign);
    auto initializer = parseExpression();
    consume(TokenKind::Semicolon);
    return std::make_unique<VarDeclStmt>(declaredType, name, std::move(initializer));
}

// ── print(expr); ──
std::unique_ptr<Stmt> Parser::parsePrint() {
    consume(TokenKind::LeftParen);
    auto expr = parseExpression();
    consume(TokenKind::RightParen);
    consume(TokenKind::Semicolon);
    return std::make_unique<PrintStmt>(std::move(expr));
}

// ── loop(i, count) { ... } ──
std::unique_ptr<Stmt> Parser::parseLoop() {
    consume(TokenKind::LeftParen);
    if (!check(TokenKind::Identifier)) {
        std::cerr << "[parser] error: expected iterator name\n"; exit(1);
    }
    std::string iterator = advance().lexeme;
    consume(TokenKind::Comma);
    auto count = parseExpression();
    consume(TokenKind::RightParen);
    consume(TokenKind::LeftBrace);

    auto loop = std::make_unique<LoopStmt>(iterator, std::move(count));
    while (!check(TokenKind::RightBrace) && !isAtEnd())
        loop->body.push_back(parseStatement());
    consume(TokenKind::RightBrace);
    return loop;
}

// ── Type parsing ──
Type* Parser::parseType() {
    Token t = advance();
    Type* base = nullptr;

    if      (t.lexeme == "int")    base = &TYPE_INT;
    else if (t.lexeme == "double") base = &TYPE_DOUBLE;
    else if (t.lexeme == "string") base = &TYPE_STRING;
    else if (t.lexeme == "bool")   base = &TYPE_BOOL;
    else if (t.lexeme == "void")   base = &TYPE_VOID;
    else if (t.lexeme == "tensor") base = &TYPE_TENSOR;
    else if (t.kind == TokenKind::Identifier) {

        base = new Type(TypeKind::Struct, t.lexeme);
    }
    else {
        std::cerr << "[parser] error: unknown type '" << t.lexeme << "'\n"; exit(1);
    }

    if (match(TokenKind::LeftBracket))
    {
        consume(TokenKind::RightBracket);
        return new Type(TypeKind::Array, base);
    }
    return base;

}

// =============================
// Expressions
// =============================

int Parser::getPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind::Star:
        case TokenKind::Slash:        return 3;
        case TokenKind::Plus:
        case TokenKind::Minus:        return 2;
        case TokenKind::Less:
        case TokenKind::LessEqual:
        case TokenKind::Greater:
        case TokenKind::GreaterEqual:
        case TokenKind::EqualEqual:
        case TokenKind::NotEqual:     return 1;
        default:                      return 0;
    }
}

std::unique_ptr<Expr> Parser::parseExpression(int precedence) {
    auto left = parsePrimary();

    // Postfix: array indexing  arr[i]
    while (match(TokenKind::LeftBracket)) {
        auto index = parseExpression();
        consume(TokenKind::RightBracket);
        left = std::make_unique<IndexExpr>(std::move(left), std::move(index));
    }

    //Postfix: member access obj.field
    while (match(TokenKind::Dot))
    {
        if (!check(TokenKind::Identifier))
        {
            std::cerr << "parser error:  expected field name after '.'\n"; exit(1);
        }
        std::string field = advance().lexeme;
        left = std::make_unique<MemberAccessExpr>(std::move(left), field);
    }

    // Infix binary operators
    while (!isAtEnd() && getPrecedence(peek().kind) > precedence) {
        Token op  = advance();
        int newPrec = getPrecedence(op.kind);
        auto right  = parseExpression(newPrec);
        left = std::make_unique<BinaryExpr>(std::move(left), op.lexeme, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (isAtEnd()) {
        std::cerr << "[parser] error: unexpected end of file\n"; exit(1);
    }

    Token t = advance();

    // ── Literals ──────────────────────────────
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

    // ── Identifier: variable OR function call ──
    // FIX: check for '(' after the name to decide which it is
    if (t.kind == TokenKind::Identifier) {
        // Struct literal:Name { field: expr, ... }
        if (check(TokenKind::LeftBrace))
        {
            advance();
            auto lit = std::make_unique<StructLiteralExpr>(t.lexeme);
            while (!check(TokenKind::RightBrace) && !isAtEnd())
            {
                if (!check(TokenKind::Identifier))
                {
                    std::cerr << "[parser] error: expected field name\n"; exit(1);
                }
                std::string fieldName = advance().lexeme;
                consume(TokenKind::Colon);
                auto val = parseExpression();
                lit->fields.push_back({fieldName, std::move(val)});
                if (!match(TokenKind::Comma)) break;
            }
            consume(TokenKind::RightBrace);
            return lit;
        }

        if (match(TokenKind::LeftParen)) {
            // Function call: name(arg, arg, ...)
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenKind::RightParen)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RightParen);
            return std::make_unique<CallExpr>(t.lexeme, std::move(args));
        }
        // Plain variable reference
        return std::make_unique<VariableExpr>(t.lexeme);
    }

    // ── Grouping: ( expr ) ────────────────────
    if (t.kind == TokenKind::LeftParen) {
        auto expr = parseExpression();
        consume(TokenKind::RightParen);
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    // ── Tensor literal: [[ ... ]] ─────────────
    if (t.kind == TokenKind::LeftBracket && check(TokenKind::LeftBracket)) {
        current--;  // back up so parseTensorLiteral sees the outer [
        return parseTensorLiteral();
    }

    // ── Array literal: [ expr, ... ] ─────────
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

    std::cerr << "[parser] error: unexpected token '"
              << t.lexeme << "' at line " << t.line
              << " col " << t.column << "\n";
    exit(1);
}

std::unique_ptr<Expr> Parser::parseTensorLiteral() {
    std::vector<std::vector<std::unique_ptr<Expr>>> rows;
    consume(TokenKind::LeftBracket);  // outer [

    while (!check(TokenKind::RightBracket) && !isAtEnd()) {
        consume(TokenKind::LeftBracket);  // inner [
        std::vector<std::unique_ptr<Expr>> row;
        if (!check(TokenKind::RightBracket)) {
            do {
                row.push_back(parseExpression());
            } while (match(TokenKind::Comma));
        }
        consume(TokenKind::RightBracket);  // inner ]
        rows.push_back(std::move(row));
        if (!match(TokenKind::Comma)) break;
    }

    consume(TokenKind::RightBracket);  // outer ]
    return std::make_unique<TensorLiteralExpr>(std::move(rows));
}