#include "CodeGen.h"
#include "LLVMContext.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>

#include <stdexcept>

/* ================= SETUP ================= */

CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

llvm::Type* CodeGen::intTy() const {
    return llvm::Type::getInt32Ty(*llvm.context);
}

/* ================= PROGRAM ================= */

void CodeGen::generate(Program& program) {
    auto* mainType =
        llvm::FunctionType::get(intTy(), false);

    auto* mainFunc =
        llvm::Function::Create(
            mainType,
            llvm::Function::ExternalLinkage,
            "main",
            llvm.module.get()
        );

    auto* entry =
        llvm::BasicBlock::Create(*llvm.context, "entry", mainFunc);

    llvm.builder->SetInsertPoint(entry);

    /* printf */
    auto* printfType =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*llvm.context),
            { llvm::PointerType::get(*llvm.context, 0) },
            true
        );

    auto printfFunc =
        llvm.module->getOrInsertFunction("printf", printfType);

    /* statements */
    for (auto& stmt : program.statements) {
        if (auto* v = dynamic_cast<VarDecl*>(stmt.get())) {
            genVarDecl(*v);
        }
        else if (auto* p = dynamic_cast<PrintStmt*>(stmt.get())) {
            genPrintStmt(*p, printfFunc);
        }
    }

    llvm.builder->CreateRet(
        llvm::ConstantInt::get(intTy(), 0)
    );

    llvm::verifyFunction(*mainFunc);
}

/* ================= STATEMENTS ================= */

void CodeGen::genVarDecl(VarDecl& decl) {
    auto* slot =
        llvm.builder->CreateAlloca(intTy(), nullptr, decl.name);

    if (decl.initializer) {
        auto* value = genExpr(*decl.initializer);
        llvm.builder->CreateStore(value, slot);
    }

    locals[decl.name] = slot;
}

void CodeGen::genPrintStmt(
    PrintStmt& stmt,
    llvm::FunctionCallee printfFunc
) {
    auto* value = genExpr(*stmt.expr);

    auto* fmt =
        llvm.builder->CreateGlobalStringPtr("%d\n");

    llvm.builder->CreateCall(
        printfFunc,
        llvm::ArrayRef<llvm::Value*>({ fmt, value })
    );
}

/* ================= EXPRESSIONS ================= */

llvm::Value* CodeGen::genExpr(Expr& expr) {

    if (auto* i = dynamic_cast<IntegerLiteral*>(&expr)) {
        return llvm::ConstantInt::get(intTy(), i->value, true);
    }

    if (auto* v = dynamic_cast<VarRef*>(&expr)) {
        auto it = locals.find(v->name);
        if (it == locals.end())
            throw std::runtime_error("Undefined variable: " + v->name);

        return llvm.builder->CreateLoad(intTy(), it->second);
    }

    if (auto* u = dynamic_cast<UnaryExpr*>(&expr)) {
        auto* val = genExpr(*u->expr);
        return llvm.builder->CreateNeg(val);
    }

    if (auto* b = dynamic_cast<BinaryExpr*>(&expr)) {
        auto* lhs = genExpr(*b->lhs);
        auto* rhs = genExpr(*b->rhs);

        switch (b->op) {
            case BinaryOp::Add: return llvm.builder->CreateAdd(lhs, rhs);
            case BinaryOp::Sub: return llvm.builder->CreateSub(lhs, rhs);
            case BinaryOp::Mul: return llvm.builder->CreateMul(lhs, rhs);
            case BinaryOp::Div: return llvm.builder->CreateSDiv(lhs, rhs);
            case BinaryOp::Mod: return llvm.builder->CreateSRem(lhs, rhs);
        }
    }

    throw std::runtime_error("Unsupported expression in codegen");
}
