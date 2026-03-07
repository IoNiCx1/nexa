#include "CodeGen.h"
#include <iostream>
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
    // Define external printf for the print statement
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

    if (type->isArray())
        return llvm::PointerType::get(llvm::Type::getInt32Ty(context), 0);

    return llvm::Type::getVoidTy(context);
}

// =============================
// Program Generation
// =============================

void CodeGen::generate(Program& program)
{
    // 1. Generate all function prototypes and bodies first
    for (auto& stmt : program.statements)
        if (dynamic_cast<FunctionDecl*>(stmt.get()))
            generateStmt(stmt.get());

    // 2. Generate the main function entry point
    auto mainType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    auto mainFunc = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module.get());
    auto entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // 3. Generate top-level statements inside main
    for (auto& stmt : program.statements)
        if (!dynamic_cast<FunctionDecl*>(stmt.get()))
            generateStmt(stmt.get());

    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    llvm::verifyFunction(*mainFunc);
}

// =============================
// Statement Generation
// =============================

void CodeGen::generateStmt(Stmt* stmt)
{
    // Function Declaration
    if (auto fn = dynamic_cast<FunctionDecl*>(stmt))
    {
        std::vector<llvm::Type*> paramTypes;
        for (auto& p : fn->params)
            paramTypes.push_back(getLLVMType(p.second));

        auto funcType = llvm::FunctionType::get(getLLVMType(fn->returnType), paramTypes, false);
        auto function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, fn->name, module.get());
        auto block = llvm::BasicBlock::Create(context, "entry", function);
        
        auto oldInsertPoint = builder.GetInsertBlock();
        builder.SetInsertPoint(block);

        auto oldNamedValues = namedValues;
        namedValues.clear();

        int idx = 0;
        for (auto& arg : function->args())
        {
            auto paramName = fn->params[idx].first;
            auto alloca = builder.CreateAlloca(arg.getType(), nullptr, paramName);
            builder.CreateStore(&arg, alloca);
            namedValues[paramName] = alloca;
            idx++;
        }

        for (auto& s : fn->body)
            generateStmt(s.get());

        // Add implicit return if missing
        if (!builder.GetInsertBlock()->getTerminator()) {
            if (fn->returnType->isDouble())
                builder.CreateRet(llvm::ConstantFP::get(context, llvm::APFloat(0.0)));
            else if (fn->returnType->isInt())
                builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
            else if (fn->returnType->isBool())
                builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 0));
            else
                builder.CreateRet(nullptr); // Void-ish
        }

        namedValues = oldNamedValues;
        if (oldInsertPoint) builder.SetInsertPoint(oldInsertPoint);
        return;
    }

    // Return
    else if (auto ret = dynamic_cast<ReturnStmt*>(stmt))
    {
        auto val = generateExpr(ret->value.get());
        builder.CreateRet(val);
    }

    // Variable Declaration
    else if (auto var = dynamic_cast<VarDeclStmt*>(stmt))
    {
        auto initVal = generateExpr(var->initializer.get());
        if (var->declaredType->isArray()) {
            namedValues[var->name] = initVal;
        } else {
            auto alloca = builder.CreateAlloca(getLLVMType(var->declaredType), nullptr, var->name);
            builder.CreateStore(initVal, alloca);
            namedValues[var->name] = alloca;
        }
    }

    // Assignment
    else if (auto assign = dynamic_cast<AssignmentStmt*>(stmt))
    {
        auto value = generateExpr(assign->value.get());
        if (auto var = dynamic_cast<VariableExpr*>(assign->target.get())) {
            builder.CreateStore(value, namedValues[var->name]);
        } else if (auto index = dynamic_cast<IndexExpr*>(assign->target.get())) {
            auto arrayPtr = generateExpr(index->array.get());
            auto idx = generateExpr(index->index.get());
            auto elementPtr = builder.CreateGEP(llvm::Type::getInt32Ty(context), arrayPtr, idx);
            builder.CreateStore(value, elementPtr);
        }
    }

    // Print
    else if (auto print = dynamic_cast<PrintStmt*>(stmt))
    {
        auto val = generateExpr(print->expression.get());
        auto type = print->expression->inferredType;
        const char* fmtStr = type->isDouble() ? "%f\n" : (type->isString() ? "%s\n" : "%d\n");
        
        auto fmt = builder.CreateGlobalStringPtr(fmtStr);
        if (type->isBool()) val = builder.CreateZExt(val, llvm::Type::getInt32Ty(context));
        builder.CreateCall(printfFunc, {fmt, val});
    }

    // Loop
    else if (auto loop = dynamic_cast<LoopStmt*>(stmt))
    {
        auto countVal = generateExpr(loop->count.get());
        auto function = builder.GetInsertBlock()->getParent();

        auto loopVar = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, loop->iterator);
        builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0), loopVar);

        namedValues[loop->iterator] = loopVar;

        auto condBlock = llvm::BasicBlock::Create(context, "loop_cond", function);
        auto bodyBlock = llvm::BasicBlock::Create(context, "loop_body", function);
        auto endBlock = llvm::BasicBlock::Create(context, "loop_end", function);

        builder.CreateBr(condBlock);
        builder.SetInsertPoint(condBlock);

        auto current = builder.CreateLoad(llvm::Type::getInt32Ty(context), loopVar);
        auto cond = builder.CreateICmpSLT(current, countVal);
        builder.CreateCondBr(cond, bodyBlock, endBlock);

        builder.SetInsertPoint(bodyBlock);
        for (auto& bodyStmt : loop->body)
            generateStmt(bodyStmt.get());

        auto nextVal = builder.CreateAdd(builder.CreateLoad(llvm::Type::getInt32Ty(context), loopVar),
                                        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1));
        builder.CreateStore(nextVal, loopVar);
        builder.CreateBr(condBlock);

        builder.SetInsertPoint(endBlock);
    }

    // If
    else if (auto ifStmt = dynamic_cast<IfStmt*>(stmt))
    {
        auto condition = generateExpr(ifStmt->condition.get());
        auto function = builder.GetInsertBlock()->getParent();

        auto thenBlock = llvm::BasicBlock::Create(context, "then", function);
        auto elseBlock = llvm::BasicBlock::Create(context, "else", function);
        auto mergeBlock = llvm::BasicBlock::Create(context, "ifcont", function);

        builder.CreateCondBr(condition, thenBlock, elseBlock);

        builder.SetInsertPoint(thenBlock);
        for (auto& s : ifStmt->thenBranch) generateStmt(s.get());
        builder.CreateBr(mergeBlock);

        builder.SetInsertPoint(elseBlock);
        for (auto& s : ifStmt->elseBranch) generateStmt(s.get());
        builder.CreateBr(mergeBlock);

        builder.SetInsertPoint(mergeBlock);
    }
}

// =============================
// Expression Generation
// =============================

llvm::Value* CodeGen::generateExpr(Expr* expr)
{
    // Function Call
    if (auto call = dynamic_cast<CallExpr*>(expr))
    {
        auto function = module->getFunction(call->callee);
        if (!function) {
            std::cerr << "Undefined function in CodeGen: " << call->callee << "\n";
            exit(1);
        }
        std::vector<llvm::Value*> args;
        for (auto& a : call->arguments)
            args.push_back(generateExpr(a.get()));
        return builder.CreateCall(function, args);
    }

    // Literals
    if (auto intLit = dynamic_cast<IntegerLiteral*>(expr))
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), intLit->value);

    if (auto dblLit = dynamic_cast<DoubleLiteral*>(expr))
        return llvm::ConstantFP::get(context, llvm::APFloat(dblLit->value));

    if (auto boolLit = dynamic_cast<BoolLiteral*>(expr))
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), boolLit->value ? 1 : 0);

    if (auto strLit = dynamic_cast<StringLiteral*>(expr))
        return builder.CreateGlobalStringPtr(strLit->value);

    // Variable Access (THE FIX IS HERE)
    if (auto var = dynamic_cast<VariableExpr*>(expr))
    {
        auto alloca = namedValues[var->name];
        if (!alloca) {
            std::cerr << "Undefined variable in CodeGen: " << var->name << "\n";
            exit(1);
        }
        // We explicitly pass the type to CreateLoad
        return builder.CreateLoad(getLLVMType(var->inferredType), alloca, var->name);
    }

    // Binary Expressions
    if (auto bin = dynamic_cast<BinaryExpr*>(expr))
    {
        auto L = generateExpr(bin->left.get());
        auto R = generateExpr(bin->right.get());
        bool isDouble = bin->left->inferredType->isDouble();

        if (bin->op == "+") return isDouble ? builder.CreateFAdd(L, R) : builder.CreateAdd(L, R);
        if (bin->op == "-") return isDouble ? builder.CreateFSub(L, R) : builder.CreateSub(L, R);
        if (bin->op == "*") return isDouble ? builder.CreateFMul(L, R) : builder.CreateMul(L, R);
        if (bin->op == "/") return isDouble ? builder.CreateFDiv(L, R) : builder.CreateSDiv(L, R);

        if (bin->op == "<")  return isDouble ? builder.CreateFCmpOLT(L, R) : builder.CreateICmpSLT(L, R);
        if (bin->op == ">")  return isDouble ? builder.CreateFCmpOGT(L, R) : builder.CreateICmpSGT(L, R);
        if (bin->op == "==") return isDouble ? builder.CreateFCmpOEQ(L, R) : builder.CreateICmpEQ(L, R);
        if (bin->op == "!=") return isDouble ? builder.CreateFCmpONE(L, R) : builder.CreateICmpNE(L, R);
    }

    // Indexing
    if (auto index = dynamic_cast<IndexExpr*>(expr))
    {
        auto arrayPtr = generateExpr(index->array.get());
        auto idx = generateExpr(index->index.get());
        auto elementPtr = builder.CreateGEP(llvm::Type::getInt32Ty(context), arrayPtr, idx);
        return builder.CreateLoad(llvm::Type::getInt32Ty(context), elementPtr);
    }

    std::cerr << "Unknown expression in CodeGen\n";
    exit(1);
    return nullptr;
}