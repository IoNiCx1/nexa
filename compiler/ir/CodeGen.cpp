#include "CodeGen.h"

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

using namespace nexa;

// =============================
// Constructor
// =============================

CodeGen::CodeGen()
    : module(std::make_unique<llvm::Module>("nexa_module", context)),
      builder(context)
{
    auto printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context),
        llvm::PointerType::get(context, 0),
        true
    );

    printfFunc = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );

    auto mallocType = llvm::FunctionType::get(
        llvm::PointerType::get(context, 0),
        llvm::Type::getInt64Ty(context),
        false
    );

    mallocFunc = llvm::Function::Create(
        mallocType,
        llvm::Function::ExternalLinkage,
        "malloc",
        module.get()
    );
}

llvm::Module* CodeGen::getModule() {
    return module.get();
}

// =============================
// Type Mapping
// =============================

llvm::Type* CodeGen::getLLVMType(Type* type)
{
    if (type->isInt())
        return llvm::Type::getInt32Ty(context);

    if (type->isDouble())
        return llvm::Type::getDoubleTy(context);

    if (type->isString())
        return llvm::PointerType::get(context, 0);

    if (type->isBool())
        return llvm::Type::getInt1Ty(context);

    if (type->kind == TypeKind::Array)
        return llvm::PointerType::get(
            llvm::Type::getInt32Ty(context),
            0
        );

    return nullptr;
}

// =============================
// Program Generation
// =============================

void CodeGen::generate(Program& program)
{
    auto mainType =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(context),
            false
        );

    auto mainFunc =
        llvm::Function::Create(
            mainType,
            llvm::Function::ExternalLinkage,
            "main",
            module.get()
        );

    auto entry =
        llvm::BasicBlock::Create(
            context,
            "entry",
            mainFunc
        );

    builder.SetInsertPoint(entry);

    for (auto& stmt : program.statements)
        generateStmt(stmt.get());

    builder.CreateRet(
        llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(context),
            0
        )
    );

    llvm::verifyFunction(*mainFunc);
}

// =============================
// Statement Generation
// =============================

void CodeGen::generateStmt(Stmt* stmt)
{
    if (auto var = dynamic_cast<VarDeclStmt*>(stmt))
    {
        auto initVal =
            generateExpr(var->initializer.get());

        if (var->declaredType->kind == TypeKind::Array)
        {
            namedValues[var->name] = initVal;
        }
        else
        {
            auto llvmType =
                getLLVMType(var->declaredType);

            auto alloca =
                builder.CreateAlloca(llvmType);

            builder.CreateStore(initVal, alloca);

            namedValues[var->name] = alloca;
        }
    }

    else if (auto assign =
        dynamic_cast<AssignmentStmt*>(stmt))
    {
        auto value =
            generateExpr(assign->value.get());

        if (auto var =
            dynamic_cast<VariableExpr*>(assign->target.get()))
        {
            auto ptr = namedValues[var->name];
            builder.CreateStore(value, ptr);
        }
    }

    else if (auto print =
        dynamic_cast<PrintStmt*>(stmt))
    {
        auto val =
            generateExpr(print->expression.get());

        auto type =
            print->expression->inferredType;

        if (type->isInt())
        {
            auto fmt =
                builder.CreateGlobalString("%d\n");
            builder.CreateCall(printfFunc, {fmt, val});
        }
        else if (type->isDouble())
        {
            auto fmt =
                builder.CreateGlobalString("%f\n");
            builder.CreateCall(printfFunc, {fmt, val});
        }
        else if (type->isString())
        {
            auto fmt =
                builder.CreateGlobalString("%s\n");
            builder.CreateCall(printfFunc, {fmt, val});
        }
        else if (type->isBool())
        {
            auto fmt =
                builder.CreateGlobalString("%d\n");

            auto extended =
                builder.CreateZExt(
                    val,
                    llvm::Type::getInt32Ty(context)
                );

            builder.CreateCall(
                printfFunc,
                {fmt, extended}
            );
        }

    else if (auto loop =
        dynamic_cast<LoopStmt*>(stmt))
    {
        auto countVal =
            generateExpr(loop->count.get());

        auto function =
            builder.GetInsertBlock()->getParent();

        auto loopVar =
            builder.CreateAlloca(
                llvm::Type::getInt32Ty(context)
            );

        builder.CreateStore(
            llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(context),
                0
            ),
            loopVar
        );

        namedValues[loop->iterator] = loopVar;

        auto condBlock =
            llvm::BasicBlock::Create(context, "loop_cond", function);

        auto bodyBlock =
            llvm::BasicBlock::Create(context, "loop_body", function);

        auto endBlock =
            llvm::BasicBlock::Create(context, "loop_end", function);

        builder.CreateBr(condBlock);

        builder.SetInsertPoint(condBlock);

        auto current =
            builder.CreateLoad(
                llvm::Type::getInt32Ty(context),
                loopVar
            );

        auto cond =
            builder.CreateICmpSLT(current, countVal);

        builder.CreateCondBr(cond, bodyBlock, endBlock);

        builder.SetInsertPoint(bodyBlock);

        for (auto& bodyStmt : loop->body)
            generateStmt(bodyStmt.get());

        auto increment =
            builder.CreateAdd(
                current,
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(context),
                    1
                )
            );

        builder.CreateStore(increment, loopVar);
        builder.CreateBr(condBlock);

        builder.SetInsertPoint(endBlock);
    }

    else if (type->isBool())
    {
        auto fmt = 
            builder.CreateGlobalString("%d\n");

        auto extended = 
            builder.CreateZExt(
                val,
                llvm::Type::getInt32Ty(context)
            );

        builder.CreateCall(
            printfFunc,
            {fmt, extended}
        );
    }
}
}

// =============================
// Expression Generation
// =============================

llvm::Value* CodeGen::generateExpr(Expr* expr)
{
    if (auto arr = dynamic_cast<ArrayLiteralExpr*>(expr))
    {
        int size = arr->elements.size();

        llvm::Type* elemType =
            llvm::Type::getInt32Ty(context);

        llvm::Value* arrayAlloc =
            builder.CreateAlloca(
                elemType,
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(context),
                    size
                )
            );

        for (int i = 0; i < size; i++)
        {
            llvm::Value* index =
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(context),
                    i
                );

            llvm::Value* elementPtr =
                builder.CreateGEP(
                    elemType,
                    arrayAlloc,
                    index
                );

            llvm::Value* value =
                generateExpr(arr->elements[i].get());

            builder.CreateStore(value, elementPtr);
        }

        return arrayAlloc;
    }

    if (auto index = dynamic_cast<IndexExpr*>(expr))
    {
        llvm::Value* arrayPtr =
            generateExpr(index->array.get());

        llvm::Value* idx =
            generateExpr(index->index.get());

        llvm::Type* elemType =
            llvm::Type::getInt32Ty(context);

        llvm::Value* elementPtr =
            builder.CreateGEP(elemType, arrayPtr, idx);

        return builder.CreateLoad(elemType, elementPtr);
    }

    if (auto intLit =
        dynamic_cast<IntegerLiteral*>(expr))
    {
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(context),
            intLit->value
        );
    }

    if (auto dblLit =
        dynamic_cast<DoubleLiteral*>(expr))
    {
        return llvm::ConstantFP::get(
            llvm::Type::getDoubleTy(context),
            dblLit->value
        );
    }

    if (auto strLit =
        dynamic_cast<StringLiteral*>(expr))
    {
        return builder.CreateGlobalString(
            strLit->value
        );
    }

    if (auto boolLit =
        dynamic_cast<BooleanLiteral*>(expr))
    {
        return llvm::ConstantInt::get(
            llvm::Type::getInt1Ty(context),
            boolLit->value
        );
    }

    if (auto var =
        dynamic_cast<VariableExpr*>(expr))
    {
        auto ptr = namedValues[var->name];

        if (expr->inferredType->kind == TypeKind::Array)
            return ptr;

        return builder.CreateLoad(
            getLLVMType(expr->inferredType),
            ptr
        );
    }

    if (auto bin =
        dynamic_cast<BinaryExpr*>(expr))
    {
        auto leftVal =
            generateExpr(bin->left.get());

        auto rightVal =
            generateExpr(bin->right.get());

        auto leftType =
            bin->left->inferredType;

        auto rightType =
            bin->right->inferredType;

        // Promote int → double if result is double
        if (expr->inferredType->isDouble())
        {
            if (leftType->isInt())
                leftVal =
                    builder.CreateSIToFP(
                        leftVal,
                        llvm::Type::getDoubleTy(context)
                    );

            if (rightType->isInt())
                rightVal =
                    builder.CreateSIToFP(
                        rightVal,
                        llvm::Type::getDoubleTy(context)
                    );
        }

        if (expr->inferredType->isInt())
        {
            if (bin->op == "+")
                return builder.CreateAdd(leftVal, rightVal);
            if (bin->op == "-")
                return builder.CreateSub(leftVal, rightVal);
            if (bin->op == "*")
                return builder.CreateMul(leftVal, rightVal);
            if (bin->op == "/")
                return builder.CreateSDiv(leftVal, rightVal);
        }

        if (expr->inferredType->isDouble())
        {
            if (bin->op == "+")
                return builder.CreateFAdd(leftVal, rightVal);
            if (bin->op == "-")
                return builder.CreateFSub(leftVal, rightVal);
            if (bin->op == "*")
                return builder.CreateFMul(leftVal, rightVal);
            if (bin->op == "/")
                return builder.CreateFDiv(leftVal, rightVal);
        }
    }

    return nullptr;
}
