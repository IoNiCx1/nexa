#include "CodeGen.h"
#include "LLVMContext.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <stdexcept>

/* ================= CONSTRUCTOR ================= */

CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

/* ================= ENTRY ================= */

void CodeGen::generate(Program& program) {

    auto* mainFn = llvm::Function::Create(
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(*llvm.context), false),
        llvm::Function::ExternalLinkage,
        "main",
        llvm.module.get());

    auto* entry =
        llvm::BasicBlock::Create(*llvm.context, "entry", mainFn);
    llvm.builder->SetInsertPoint(entry);

    auto printfFunc =
        llvm.module->getOrInsertFunction(
            "printf",
            llvm::FunctionType::get(
                llvm::Type::getInt32Ty(*llvm.context),
                { llvm::PointerType::get(*llvm.context, 0) },
                true));

    for (auto& stmt : program.statements)
        genStatement(*stmt, printfFunc);

    llvm.builder->CreateRet(
        llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(*llvm.context), 0));

    llvm::verifyFunction(*mainFn);
}

/* ================= STATEMENTS ================= */

void CodeGen::genStatement(Stmt& stmt,
                           llvm::FunctionCallee printfFunc) {

    /* -------- PRINT -------- */
    if (auto* p = dynamic_cast<PrintStmt*>(&stmt)) {

        Expr* expr = p->expr.get();

        /* ---- print <a> ---- */
        if (auto* t = dynamic_cast<TypedRefExpr*>(expr)) {

            if (t->type == TypeKind::IntArray) {
                auto* arr = variables[t->name];
                auto* arrType =
                    llvm::cast<llvm::ArrayType>(
                        llvm::cast<llvm::AllocaInst>(arr)
                            ->getAllocatedType());

                auto count = arrType->getNumElements();

                auto* open =
                    llvm.builder->CreateGlobalStringPtr("[ ");
                auto* elemFmt =
                    llvm.builder->CreateGlobalStringPtr("%d ");
                auto* close =
                    llvm.builder->CreateGlobalStringPtr("]\n");

                llvm.builder->CreateCall(printfFunc, { open });

                for (unsigned i = 0; i < count; ++i) {
                    auto* ptr =
                        llvm.builder->CreateInBoundsGEP(
                            arrType,
                            arr,
                            { llvm.builder->getInt32(0),
                              llvm.builder->getInt32(i) });

                    auto* val =
                        llvm.builder->CreateLoad(
                            llvm::Type::getInt32Ty(*llvm.context),
                            ptr);

                    llvm.builder->CreateCall(
                        printfFunc, { elemFmt, val });
                }

                llvm.builder->CreateCall(printfFunc, { close });
                return;
            }
        }

        /* ---- print s / print x ---- */
        if (auto* v = dynamic_cast<VarRef*>(expr)) {

            if (varTypes[v->name] == TypeKind::String) {
                auto* fmt =
                    llvm.builder->CreateGlobalStringPtr("%s\n");
                llvm.builder->CreateCall(
                    printfFunc, { fmt, variables[v->name] });
                return;
            }

            if (varTypes[v->name] == TypeKind::IntArray) {
                auto* arr = variables[v->name];
                auto* arrType =
                    llvm::cast<llvm::ArrayType>(
                        llvm::cast<llvm::AllocaInst>(arr)
                            ->getAllocatedType());

                auto count = arrType->getNumElements();

                auto* open =
                    llvm.builder->CreateGlobalStringPtr("[ ");
                auto* elemFmt =
                    llvm.builder->CreateGlobalStringPtr("%d ");
                auto* close =
                    llvm.builder->CreateGlobalStringPtr("]\n");

                llvm.builder->CreateCall(printfFunc, { open });

                for (unsigned i = 0; i < count; ++i) {
                    auto* ptr =
                        llvm.builder->CreateInBoundsGEP(
                            arrType,
                            arr,
                            { llvm.builder->getInt32(0),
                              llvm.builder->getInt32(i) });

                    auto* val =
                        llvm.builder->CreateLoad(
                            llvm::Type::getInt32Ty(*llvm.context),
                            ptr);

                    llvm.builder->CreateCall(
                        printfFunc, { elemFmt, val });
                }

                llvm.builder->CreateCall(printfFunc, { close });
                return;
            }

            auto* fmt =
                llvm.builder->CreateGlobalStringPtr("%d\n");
            auto* val =
                llvm.builder->CreateLoad(
                    llvm::Type::getInt32Ty(*llvm.context),
                    variables[v->name]);
            llvm.builder->CreateCall(printfFunc, { fmt, val });
            return;
        }
    }

    /* -------- VARIABLE DECL -------- */
    if (auto* v = dynamic_cast<VarDecl*>(&stmt)) {

        if (v->type.kind == TypeKind::String) {
            auto* val = genExpression(*v->init);
            variables[v->name] = val;
            varTypes[v->name] = TypeKind::String;
            return;
        }

        auto* alloc =
            llvm.builder->CreateAlloca(
                llvm::Type::getInt32Ty(*llvm.context),
                nullptr,
                v->name);

        auto* init = genExpression(*v->init);
        llvm.builder->CreateStore(init, alloc);

        variables[v->name] = alloc;
        varTypes[v->name] = TypeKind::Int;
        return;
    }

    /* -------- ARRAY DECL -------- */
    if (auto* a = dynamic_cast<ArrayDecl*>(&stmt)) {

        auto count = a->elements.size();
        auto* arrType =
            llvm::ArrayType::get(
                llvm::Type::getInt32Ty(*llvm.context),
                count);

        auto* alloc =
            llvm.builder->CreateAlloca(arrType, nullptr, a->name);

        llvm::Constant* zero =
            llvm::ConstantAggregateZero::get(arrType);
        llvm.builder->CreateStore(zero, alloc);

        for (size_t i = 0; i < count; ++i) {
            auto* val = genExpression(*a->elements[i]);
            auto* ptr =
                llvm.builder->CreateInBoundsGEP(
                    arrType,
                    alloc,
                    { llvm.builder->getInt32(0),
                      llvm.builder->getInt32(i) });

            llvm.builder->CreateStore(val, ptr);
        }

        variables[a->name] = alloc;
        varTypes[a->name] = TypeKind::IntArray;
        return;
    }
}

/* ================= EXPRESSIONS ================= */

llvm::Value* CodeGen::genExpression(Expr& expr) {

    if (auto* i = dynamic_cast<IntegerLiteral*>(&expr))
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(*llvm.context), i->value);

    if (auto* s = dynamic_cast<StringLiteral*>(&expr))
        return llvm.builder->CreateGlobalStringPtr(s->value);

    if (auto* v = dynamic_cast<VarRef*>(&expr))
        return variables[v->name];

    throw std::runtime_error("Unsupported expression in codegen");
}
