#include "CodeGen.h"
#include "LLVMContext.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>

#include <stdexcept>
#include <vector>

/* ================= SETUP ================= */

CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

llvm::Type* CodeGen::intType() const {
    return llvm::Type::getInt32Ty(*llvm.context);
}

/* ================= PROGRAM ================= */

void CodeGen::generate(Program& program) {
    llvm::FunctionType* mainType =
        llvm::FunctionType::get(intType(), false);

    llvm::Function* mainFunc =
        llvm::Function::Create(
            mainType,
            llvm::Function::ExternalLinkage,
            "main",
            llvm.module.get()
        );

    llvm::BasicBlock* entry =
        llvm::BasicBlock::Create(*llvm.context, "entry", mainFunc);

    llvm.builder->SetInsertPoint(entry);

    /* printf */
    llvm::FunctionType* printfType =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*llvm.context),
            { llvm::PointerType::get(*llvm.context, 0) },
            true
        );

    llvm::FunctionCallee printfFunc =
        llvm.module->getOrInsertFunction("printf", printfType);

    /* statements */
    for (auto& stmt : program.statements) {
        if (auto* v = dynamic_cast<VarDecl*>(stmt.get())) {
            genVarDecl(*v);
        } else if (auto* a = dynamic_cast<ArrayDecl*>(stmt.get())) {
            genArrayDecl(*a);
        } else if (auto* p = dynamic_cast<PrintStmt*>(stmt.get())) {
            genPrintStmt(*p, printfFunc);
        }
    }

    llvm.builder->CreateRet(
        llvm::ConstantInt::get(intType(), 0)
    );

    llvm::verifyFunction(*mainFunc);
}

/* ================= STATEMENTS ================= */

void CodeGen::genVarDecl(VarDecl& decl) {
    llvm::AllocaInst* alloca =
        llvm.builder->CreateAlloca(intType(), nullptr, decl.name);

    if (decl.initializer) {
        llvm::Value* initVal = genExpression(*decl.initializer);
        llvm.builder->CreateStore(initVal, alloca);
    }

    namedValues[decl.name] = alloca;
}

void CodeGen::genArrayDecl(ArrayDecl& decl) {
    size_t count = decl.elements.empty() ? 1 : decl.elements.size();

    llvm::ArrayType* arrType =
        llvm::ArrayType::get(intType(), count);

    llvm::AllocaInst* alloca =
        llvm.builder->CreateAlloca(arrType, nullptr, decl.name);

    namedValues[decl.name] = alloca;

    for (size_t i = 0; i < decl.elements.size(); ++i) {
        llvm::Value* elem =
            genExpression(*decl.elements[i]);

        llvm::Value* ptr =
            llvm.builder->CreateInBoundsGEP(
                arrType,
                alloca,
                {
                    llvm.builder->getInt32(0),
                    llvm.builder->getInt32(i)
                }
            );

        llvm.builder->CreateStore(elem, ptr);
    }
}

void CodeGen::genPrintStmt(
    PrintStmt& stmt,
    llvm::FunctionCallee& printfFunc
) {
    llvm::Value* val = genExpression(*stmt.expr);

    llvm::Value* fmt =
        llvm.builder->CreateGlobalStringPtr("%d\n");

    llvm.builder->CreateCall(printfFunc, { fmt, val });
}

/* ================= EXPRESSIONS ================= */

llvm::Value* CodeGen::genExpression(Expr& expr) {

    /* integer literal */
    if (auto* i = dynamic_cast<IntegerLiteral*>(&expr)) {
        return llvm::ConstantInt::get(
            intType(), i->value, true
        );
    }

    /* variable reference */
    if (auto* v = dynamic_cast<VarRef*>(&expr)) {
        auto it = namedValues.find(v->name);
        if (it == namedValues.end())
            throw std::runtime_error("Undefined variable: " + v->name);

        return llvm.builder->CreateLoad(
            intType(),
            it->second,
            v->name
        );
    }

    /* unary expression */
    if (auto* u = dynamic_cast<UnaryExpr*>(&expr)) {
        llvm::Value* operand = genExpression(*u->expr);

        switch (u->op) {
            case UnaryOp::Negate:
                return llvm.builder->CreateNeg(operand);
        }
    }

    /* binary expression */
    if (auto* b = dynamic_cast<BinaryExpr*>(&expr)) {
        llvm::Value* lhs = genExpression(*b->lhs);
        llvm::Value* rhs = genExpression(*b->rhs);

        switch (b->op) {
            case BinaryOp::Add:
                return llvm.builder->CreateAdd(lhs, rhs);
            case BinaryOp::Sub:
                return llvm.builder->CreateSub(lhs, rhs);
            case BinaryOp::Mul:
                return llvm.builder->CreateMul(lhs, rhs);
            case BinaryOp::Div:
                return llvm.builder->CreateSDiv(lhs, rhs);
            case BinaryOp::Mod:
                return llvm.builder->CreateSRem(lhs, rhs);
        }
    }

    throw std::runtime_error("Unsupported expression in codegen");
}
