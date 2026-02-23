#include "CodeGen.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/Support/raw_ostream.h>

using namespace nexa;

CodeGen::CodeGen()
    : module(std::make_unique<llvm::Module>("nexa_module", context)),
      builder(context) {

    llvm::Type *i8Type = llvm::Type::getInt8Ty(context);
    llvm::PointerType *i8PtrType = llvm::PointerType::get(i8Type, 0);

    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(i8PtrType);

    llvm::FunctionType *printfType =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(context),
            printfArgs,
            true
        );

    printfFunc = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );
}

llvm::Module* CodeGen::getModule() {
    return module.get();
}

llvm::Type* CodeGen::getLLVMType(const Type &type) {

    switch (type.kind) {
        case TypeKind::Int:
            return llvm::Type::getInt32Ty(context);

        case TypeKind::Double:
            return llvm::Type::getDoubleTy(context);

        case TypeKind::String:
            return llvm::PointerType::get(
                llvm::Type::getInt8Ty(context),
                0
            );

        default:
            return nullptr;
    }
}

llvm::Value* CodeGen::promoteIfNeeded(
    llvm::Value *value,
    const Type &from,
    const Type &to) {

    if (from == to)
        return value;

    if (from.isInt() && to.isDouble())
        return builder.CreateSIToFP(
            value,
            llvm::Type::getDoubleTy(context)
        );

    return value;
}

llvm::Value* CodeGen::generateExpr(Expr *expr) {

    if (auto intLit = dynamic_cast<IntegerLiteral*>(expr)) {
        return llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(context),
            intLit->value
        );
    }

    if (auto dblLit = dynamic_cast<DoubleLiteral*>(expr)) {
        return llvm::ConstantFP::get(
            llvm::Type::getDoubleTy(context),
            dblLit->value
        );
    }

    if (auto strLit = dynamic_cast<StringLiteral*>(expr)) {
        return builder.CreateGlobalStringPtr(strLit->value);
    }

    if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        return builder.CreateLoad(
            getLLVMType(expr->inferredType),
            namedValues[var->name]
        );
    }

    if (auto unary = dynamic_cast<UnaryExpr*>(expr)) {

        llvm::Value *operand =
            generateExpr(unary->operand.get());

        if (unary->op == "-") {

            if (expr->inferredType.isInt())
                return builder.CreateNeg(operand);
            else
                return builder.CreateFNeg(operand);
        }
    }

    if (auto binary = dynamic_cast<BinaryExpr*>(expr)) {

        llvm::Value *left =
            generateExpr(binary->left.get());

        llvm::Value *right =
            generateExpr(binary->right.get());

        Type leftType =
            binary->left->inferredType;

        Type rightType =
            binary->right->inferredType;

        Type resultType =
            expr->inferredType;

        left = promoteIfNeeded(left, leftType, resultType);
        right = promoteIfNeeded(right, rightType, resultType);

        if (resultType.isInt()) {

            if (binary->op == "+")
                return builder.CreateAdd(left, right);

            if (binary->op == "-")
                return builder.CreateSub(left, right);

            if (binary->op == "*")
                return builder.CreateMul(left, right);

            if (binary->op == "/")
                return builder.CreateSDiv(left, right);
        }
        else {

            if (binary->op == "+")
                return builder.CreateFAdd(left, right);

            if (binary->op == "-")
                return builder.CreateFSub(left, right);

            if (binary->op == "*")
                return builder.CreateFMul(left, right);

            if (binary->op == "/")
                return builder.CreateFDiv(left, right);
        }
    }

    return nullptr;
}

void CodeGen::generateStmt(Stmt *stmt) {

    if (auto varDecl =
        dynamic_cast<VarDeclStmt*>(stmt)) {

        llvm::Type *llvmType =
            getLLVMType(varDecl->declaredType);

        llvm::Value *alloca =
            builder.CreateAlloca(
                llvmType,
                nullptr,
                varDecl->name
            );

        llvm::Value *init =
            generateExpr(varDecl->initializer.get());

        init = promoteIfNeeded(
            init,
            varDecl->initializer->inferredType,
            varDecl->declaredType
        );

        builder.CreateStore(init, alloca);

        namedValues[varDecl->name] = alloca;
    }

    if (auto printStmt =
        dynamic_cast<PrintStmt*>(stmt)) {

        llvm::Value *value =
            generateExpr(printStmt->expression.get());

        Type exprType =
            printStmt->expression->inferredType;

        llvm::Value *formatStr = nullptr;

        if (exprType.isInt())
            formatStr = builder.CreateGlobalStringPtr("%d\n");
        else if (exprType.isDouble())
            formatStr = builder.CreateGlobalStringPtr("%f\n");
        else if (exprType.isString())
            formatStr = builder.CreateGlobalStringPtr("%s\n");

        builder.CreateCall(
            printfFunc,
            {formatStr, value}
        );
    }
}

void CodeGen::generate(Program &program) {

    llvm::FunctionType *mainType =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(context),
            false
        );

    llvm::Function *mainFunc =
        llvm::Function::Create(
            mainType,
            llvm::Function::ExternalLinkage,
            "main",
            module.get()
        );

    llvm::BasicBlock *entry =
        llvm::BasicBlock::Create(
            context,
            "entry",
            mainFunc
        );

    builder.SetInsertPoint(entry);

    for (auto &stmt : program.statements)
        generateStmt(stmt.get());

    builder.CreateRet(
        llvm::ConstantInt::get(
            llvm::Type::getInt32Ty(context),
            0
        )
    );

    llvm::verifyFunction(*mainFunc);
}