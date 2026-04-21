#include "SemanticAnalyzer.h"
#include <iostream>
#include <stdexcept>

using namespace nexa;

static nexa::Type TYPE_INT_ARRAY(nexa::TypeKind::Array, &nexa::TYPE_INT);

std::map<std::string, Type*> structTypeCache;

void SemanticAnalyzer::analyze(Program& program) {
    pushScope();
    for (auto& stmt : program.statements)
        checkStmt(stmt.get());
    popScope();
}

void SemanticAnalyzer::checkStmt(Stmt* stmt) {
    if (!stmt) return;

    // ── Variable declaration ──────────────────
    if (auto varDecl = dynamic_cast<VarDeclStmt*>(stmt)) {
        if (varDecl->initializer)
            checkExpr(varDecl->initializer.get());
        declare(varDecl->name, varDecl->declaredType);
        return;
    }

    // ── Imp (module import) ───────────────────
    if (dynamic_cast<ImpStmt*>(stmt)) return;

    // ── File statement ────────────────────────
    if (auto fs = dynamic_cast<FileStmt*>(stmt)) {
        if (fs->expr) {
            if (fs->expr->path)    checkExpr(fs->expr->path.get());
            if (fs->expr->content) checkExpr(fs->expr->content.get());
        }
        return;
    }

    // ── Print ─────────────────────────────────
    if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
        checkExpr(print->expression.get());
        return;
    }

    // ── Assignment ────────────────────────────
    if (auto assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        checkExpr(assign->value.get());
        if (auto varExpr = dynamic_cast<VariableExpr*>(assign->target.get())) {
            Type* t = lookup(varExpr->name);
            if (!t)
                std::cerr << "[sema] error: assignment to undeclared variable '"
                          << varExpr->name << "'\n";
            varExpr->inferredType = t;
        }
        return;
    }

    // ── Loop ──────────────────────────────────
    if (auto loop = dynamic_cast<LoopStmt*>(stmt)) {
        checkExpr(loop->count.get());
        pushScope();
        declare(loop->iterator, &TYPE_INT);
        for (auto& s : loop->body) checkStmt(s.get());
        popScope();
        return;
    }

    // ── If / else ─────────────────────────────
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        checkExpr(ifStmt->condition.get());
        pushScope();
        for (auto& s : ifStmt->thenBranch) checkStmt(s.get());
        popScope();
        pushScope();
        for (auto& s : ifStmt->elseBranch) checkStmt(s.get());
        popScope();
        return;
    }

    // ── Function declaration ──────────────────
    if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {
        declare(fn->name, fn->returnType);
        pushScope();
        for (auto& [paramName, paramType] : fn->params)
            declare(paramName, paramType);
        for (auto& s : fn->body) checkStmt(s.get());
        popScope();
        return;
    }

    // ── Return ────────────────────────────────
    if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
        if (ret->value) checkExpr(ret->value.get());
        return;
    }

    // ── Struct declaration ────────────────────
    if (auto sd = dynamic_cast<StructDecl*>(stmt)) {
        structRegistry[sd->name] = sd->fields;
        if (structTypeCache.find(sd->name) == structTypeCache.end())
            structTypeCache[sd->name] = new Type(TypeKind::Struct, sd->name);
        return;
    }

    // ── Self assignment ───────────────────────
    if (auto sa = dynamic_cast<SelfAssignmentStmt*>(stmt)) {
        checkExpr(sa->value.get());
        return;
    }

    std::cerr << "[sema] warning: unhandled statement type: "
              << typeid(*stmt).name() << "\n";
}

void SemanticAnalyzer::checkExpr(Expr* expr) {
    if (!expr) return;

    // ── Literals ──────────────────────────────
    if (dynamic_cast<IntegerLiteral*>(expr))  { expr->inferredType = &TYPE_INT;    return; }
    if (dynamic_cast<DoubleLiteral*>(expr))   { expr->inferredType = &TYPE_DOUBLE; return; }
    if (dynamic_cast<StringLiteral*>(expr))   { expr->inferredType = &TYPE_STRING; return; }
    if (dynamic_cast<BoolLiteral*>(expr))     { expr->inferredType = &TYPE_BOOL;   return; }

    // ── Constructor call ──────────────────────
    if (auto cc = dynamic_cast<ConstructorCallExpr*>(expr)) {
        for (auto& arg : cc->arguments) checkExpr(arg.get());
        expr->inferredType = new Type(TypeKind::Struct, cc->structName);
        return;
    }

    // ── Grouping ──────────────────────────────
    if (auto group = dynamic_cast<GroupingExpr*>(expr)) {
        checkExpr(group->expression.get());
        expr->inferredType = group->expression->inferredType;
        return;
    }

    // ── Array literal ─────────────────────────
    if (auto arrLit = dynamic_cast<ArrayLiteralExpr*>(expr)) {
        for (auto& elem : arrLit->elements) checkExpr(elem.get());
        expr->inferredType = &TYPE_INT_ARRAY;
        return;
    }

    // ── Array index ───────────────────────────
    if (auto idxExpr = dynamic_cast<IndexExpr*>(expr)) {
        checkExpr(idxExpr->array.get());
        checkExpr(idxExpr->index.get());
        expr->inferredType = &TYPE_INT;
        return;
    }

    // ── Tensor literal ────────────────────────
    if (auto tensorLit = dynamic_cast<TensorLiteralExpr*>(expr)) {
        for (auto& row : tensorLit->rows)
            for (auto& elem : row) checkExpr(elem.get());
        expr->inferredType = &TYPE_TENSOR;
        return;
    }

    // ── Variable reference ────────────────────
    if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        Type* t = lookup(var->name);
        if (!t) {
            std::cerr << "[sema] error: undefined variable '" << var->name << "'\n";
            expr->inferredType = &TYPE_INT;
        } else {
            expr->inferredType = t;
        }
        return;
    }

    // ── Binary expression ─────────────────────
    if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        checkExpr(bin->left.get());
        checkExpr(bin->right.get());

        auto* lType = bin->left->inferredType;
        auto* rType = bin->right->inferredType;

        if (!lType && !rType) { expr->inferredType = &TYPE_INT; return; }
        if (!lType) lType = rType;
        if (!rType) rType = lType;

        if      (lType->isTensor() || rType->isTensor()) expr->inferredType = &TYPE_TENSOR;
        else if (lType->isDouble()  || rType->isDouble()) expr->inferredType = &TYPE_DOUBLE;
        else                                              expr->inferredType = &TYPE_INT;
        return;
    }

    // ── Function call ─────────────────────────
    if (auto call = dynamic_cast<CallExpr*>(expr)) {
        for (auto& arg : call->arguments) checkExpr(arg.get());

        const std::string& fn = call->callee;

        // CSV functions — return types
        if (fn == "read_csv")  { expr->inferredType = &TYPE_TENSOR; return; }
        if (fn == "csv_row")   { expr->inferredType = &TYPE_TENSOR; return; }
        if (fn == "csv_col")   { expr->inferredType = &TYPE_TENSOR; return; }
        if (fn == "csv_slice") { expr->inferredType = &TYPE_TENSOR; return; }
        if (fn == "csv_get")   { expr->inferredType = &TYPE_DOUBLE; return; }
        if (fn == "csv_rows")  { expr->inferredType = &TYPE_INT;    return; }
        if (fn == "csv_cols")  { expr->inferredType = &TYPE_INT;    return; }
        if (fn == "write_csv") { expr->inferredType = &TYPE_VOID;   return; }
        if (fn == "csv_set")   { expr->inferredType = &TYPE_VOID;   return; }

        // Tensor/AI functions
        if (fn == "zeros"   || fn == "ones"    || fn == "reshape" ||
            fn == "shape"   || fn == "matmul")
            { expr->inferredType = &TYPE_TENSOR; return; }
        if (fn == "sum"     || fn == "mean"    || fn == "max"     ||
            fn == "min"     || fn == "get_value")
            { expr->inferredType = &TYPE_DOUBLE; return; }

        // User-defined or unknown — look up in symbol table
        Type* retType = lookup(fn);
        expr->inferredType = retType ? retType : &TYPE_INT;
        return;
    }

    // ── File expression ───────────────────────
    if (auto fe = dynamic_cast<FileExpr*>(expr)) {
        if (fe->path)    checkExpr(fe->path.get());
        if (fe->content) checkExpr(fe->content.get());
        expr->inferredType = (fe->mode == FileMode::Read) ? &TYPE_STRING : &TYPE_VOID;
        return;
    }

    // ── Struct literal ────────────────────────
    if (auto sl = dynamic_cast<StructLiteralExpr*>(expr)) {
        for (auto& f : sl->fields) checkExpr(f.second.get());
        if (structTypeCache.count(sl->structName))
            expr->inferredType = structTypeCache[sl->structName];
        else {
            Type* t = new Type(TypeKind::Struct, sl->structName);
            structTypeCache[sl->structName] = t;
            expr->inferredType = t;
        }
        return;
    }

    // ── Member access ─────────────────────────
    if (auto ma = dynamic_cast<MemberAccessExpr*>(expr)) {
        checkExpr(ma->object.get());
        Type* objType = ma->object->inferredType;
        if (!objType || !objType->isStruct()) {
            std::cerr << "[sema] error: member access on non-struct type\n";
            expr->inferredType = &TYPE_INT;
            return;
        }
        auto it = structRegistry.find(objType->structName);
        if (it == structRegistry.end()) {
            std::cerr << "[sema] error: unknown struct '" << objType->structName << "'\n";
            expr->inferredType = &TYPE_INT;
            return;
        }
        for (auto& [fieldName, fieldType] : it->second) {
            if (fieldName == ma->field) {
                expr->inferredType = fieldType ? fieldType : &TYPE_INT;
                return;
            }
        }
        std::cerr << "[sema] error: unknown field '" << ma->field
                  << "' in struct '" << objType->structName << "'\n";
        expr->inferredType = &TYPE_INT;
        return;
    }

    std::cerr << "[sema] warning: unhandled expression type: "
              << typeid(*expr).name() << " — defaulting to int\n";
    expr->inferredType = &TYPE_INT;
}