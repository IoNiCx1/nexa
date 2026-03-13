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

    module->setTargetTriple("x86_64-pc-linux-gnu");

    // -----------------------------
    // printf (Standard I/O)
    // -----------------------------

    auto printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context),
        llvm::PointerType::get(context,0),
        true
    );

    printfFunc = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        module.get()
    );

    // =============================
    // AI Runtime Functions
    // =============================

    auto matrixType = llvm::FunctionType::get(
        llvm::PointerType::get(context,0),
        {
            llvm::Type::getInt32Ty(context),
            llvm::Type::getInt32Ty(context)
        },
        false
    );

    aiMatrixFunc = llvm::Function::Create(
        matrixType,
        llvm::Function::ExternalLinkage,
        "ai_create_matrix",
        module.get()
    );

    auto setValType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {
            llvm::PointerType::get(context, 0),    
            llvm::Type::getInt32Ty(context),       
            llvm::Type::getInt32Ty(context),       
            llvm::Type::getFloatTy(context)        
        },
        false
    );

    llvm::Function::Create(
        setValType,
        llvm::Function::ExternalLinkage,
        "ai_set_value",
        module.get()
    );

    auto matmulType = llvm::FunctionType::get(
        llvm::PointerType::get(context,0),
        {
            llvm::PointerType::get(context,0),
            llvm::PointerType::get(context,0)
        },
        false
    );

    aiMatmulFunc = llvm::Function::Create(
        matmulType,
        llvm::Function::ExternalLinkage,
        "ai_matmul",
        module.get()
    );

    auto printTensorType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        {
            llvm::PointerType::get(context,0)
        },
        false
    );

    aiPrintTensorFunc = llvm::Function::Create(
        printTensorType,
        llvm::Function::ExternalLinkage,
        "ai_print",
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

    if(type->isInt())
        return llvm::Type::getInt32Ty(context);

    if(type->isDouble())
        return llvm::Type::getDoubleTy(context);

    if(type->isBool())
        return llvm::Type::getInt1Ty(context);

    if(type->isString())
        return llvm::PointerType::get(context,0);

    if(type->isTensor())
        return llvm::PointerType::get(context, 0); 

    if(type->isArray())
        return llvm::PointerType::get(
            llvm::Type::getInt32Ty(context),
            0
        );

    return llvm::Type::getVoidTy(context);

}

// =============================
// Program Generation
// =============================

void CodeGen::generate(Program& program)
{

    for(auto& stmt : program.statements)
        if(dynamic_cast<FunctionDecl*>(stmt.get()))
            generateStmt(stmt.get());

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

    for(auto& stmt : program.statements)
        if(!dynamic_cast<FunctionDecl*>(stmt.get()))
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

    // Function Declaration
    if(auto fn = dynamic_cast<FunctionDecl*>(stmt))
    {

        std::vector<llvm::Type*> paramTypes;

        for(auto& p : fn->params)
            paramTypes.push_back(getLLVMType(p.second));

        auto funcType =
            llvm::FunctionType::get(
                getLLVMType(fn->returnType),
                paramTypes,
                false
            );

        auto function =
            llvm::Function::Create(
                funcType,
                llvm::Function::ExternalLinkage,
                fn->name,
                module.get()
            );

        auto block =
            llvm::BasicBlock::Create(
                context,
                "entry",
                function
            );

        auto oldInsert = builder.GetInsertBlock();
        builder.SetInsertPoint(block);

        auto oldValues = namedValues;
        namedValues.clear();

        int idx = 0;

        for(auto& arg : function->args())
        {

            auto paramName = fn->params[idx].first;

            auto alloca =
                builder.CreateAlloca(
                    arg.getType(),
                    nullptr,
                    paramName
                );

            builder.CreateStore(&arg, alloca);

            namedValues[paramName] = alloca;

            idx++;

        }

        for(auto& s : fn->body)
            generateStmt(s.get());

        if(!builder.GetInsertBlock()->getTerminator())
        {

            if(fn->returnType->isDouble())
                builder.CreateRet(
                    llvm::ConstantFP::get(
                        context,
                        llvm::APFloat(0.0)
                    )
                );

            else if(fn->returnType->isInt())
                builder.CreateRet(
                    llvm::ConstantInt::get(
                        llvm::Type::getInt32Ty(context),
                        0
                    )
                );

            else
                builder.CreateRetVoid();

        }

        namedValues = oldValues;

        if(oldInsert)
            builder.SetInsertPoint(oldInsert);

        return;

    }

    // Return
    else if(auto ret = dynamic_cast<ReturnStmt*>(stmt))
    {
        auto val = generateExpr(ret->value.get());
        builder.CreateRet(val);
    }

    // Variable Declaration
    else if(auto var = dynamic_cast<VarDeclStmt*>(stmt))
    {

        auto initVal = generateExpr(var->initializer.get());

        auto alloca =
            builder.CreateAlloca(
                getLLVMType(var->declaredType),
                nullptr,
                var->name
            );

        builder.CreateStore(initVal, alloca);

        namedValues[var->name] = alloca;

    }

    // Assignment
    else if(auto assign = dynamic_cast<AssignmentStmt*>(stmt))
    {

        auto value = generateExpr(assign->value.get());

        if(auto var = dynamic_cast<VariableExpr*>(assign->target.get()))
        {

            builder.CreateStore(
                value,
                namedValues[var->name]
            );

        }

    }

    // Print
    else if(auto print = dynamic_cast<PrintStmt*>(stmt))
    {

        auto val = generateExpr(print->expression.get());
        auto type = print->expression->inferredType;

        if (type->isTensor()) {
            builder.CreateCall(aiPrintTensorFunc, {val});
        } else {
            const char* fmt =
                type->isDouble() ? "%f\n" :
                type->isString() ? "%s\n" :
                "%d\n";

            auto fmtVal = builder.CreateGlobalStringPtr(fmt);

            if(type->isBool())
                val = builder.CreateZExt(
                    val,
                    llvm::Type::getInt32Ty(context)
                );

            builder.CreateCall(
                printfFunc,
                {fmtVal,val}
            );
        }
    }

    // Loop Statement: loop(i, count) { ... }
    else if(auto loop = dynamic_cast<LoopStmt*>(stmt))
    {

        auto countVal = generateExpr(loop->count.get());
        auto function = builder.GetInsertBlock()->getParent();

        auto loopVar = builder.CreateAlloca(
            llvm::Type::getInt32Ty(context),
            nullptr,
            loop->iterator
        );

        builder.CreateStore(
            llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(context),
                0
            ),
            loopVar
        );

        namedValues[loop->iterator] = loopVar;

        auto condBlock = llvm::BasicBlock::Create(context,"loop_cond",function);
        auto bodyBlock = llvm::BasicBlock::Create(context,"loop_body",function);
        auto endBlock  = llvm::BasicBlock::Create(context,"loop_end",function);

        builder.CreateBr(condBlock);

        builder.SetInsertPoint(condBlock);

        auto current = builder.CreateLoad(
            llvm::Type::getInt32Ty(context),
            loopVar
        );

        auto cond = builder.CreateICmpSLT(current,countVal);

        builder.CreateCondBr(cond,bodyBlock,endBlock);

        builder.SetInsertPoint(bodyBlock);

        for(auto& s : loop->body)
            generateStmt(s.get());

        auto next =
            builder.CreateAdd(
                builder.CreateLoad(
                    llvm::Type::getInt32Ty(context),
                    loopVar
                ),
                llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(context),
                    1
                )
            );

        builder.CreateStore(next,loopVar);

        builder.CreateBr(condBlock);

        builder.SetInsertPoint(endBlock);

    }

    // If Statement
    else if(auto ifStmt = dynamic_cast<IfStmt*>(stmt))
    {

        auto condVal = generateExpr(ifStmt->condition.get());
        auto function = builder.GetInsertBlock()->getParent();

        auto thenBlock = llvm::BasicBlock::Create(context,"then",function);
        auto elseBlock = llvm::BasicBlock::Create(context,"else",function);
        auto mergeBlock = llvm::BasicBlock::Create(context,"ifcont",function);

        builder.CreateCondBr(condVal,thenBlock,elseBlock);

        builder.SetInsertPoint(thenBlock);

        for(auto& s : ifStmt->thenBranch)
            generateStmt(s.get());

        builder.CreateBr(mergeBlock);

        builder.SetInsertPoint(elseBlock);

        for(auto& s : ifStmt->elseBranch)
            generateStmt(s.get());

        builder.CreateBr(mergeBlock);

        builder.SetInsertPoint(mergeBlock);

    }

}

// =============================
// Expression Generation
// =============================

llvm::Value* CodeGen::generateExpr(Expr* expr)
{

    // Literals
    if(auto intLit = dynamic_cast<IntegerLiteral*>(expr))
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), intLit->value);

    if(auto dblLit = dynamic_cast<DoubleLiteral*>(expr))
        return llvm::ConstantFP::get(context, llvm::APFloat(dblLit->value));

    if(auto boolLit = dynamic_cast<BoolLiteral*>(expr))
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), boolLit->value ? 1 : 0);

    if(auto strLit = dynamic_cast<StringLiteral*>(expr))
        return builder.CreateGlobalStringPtr(strLit->value);

    // Grouping: (5 + 3)
    if(auto group = dynamic_cast<GroupingExpr*>(expr))
        return generateExpr(group->expression.get());

    // Array Indexing: arr[i]
    if(auto idxExpr = dynamic_cast<IndexExpr*>(expr))
    {
        auto arr = generateExpr(idxExpr->array.get());
        auto idx = generateExpr(idxExpr->index.get());
        auto ptr = builder.CreateGEP(llvm::Type::getInt32Ty(context), arr, idx);
        return builder.CreateLoad(llvm::Type::getInt32Ty(context), ptr);
    }

    // Array Literal: [10, 20, 30]
    if(auto arrLit = dynamic_cast<ArrayLiteralExpr*>(expr))
    {
        int size = arrLit->elements.size();
        auto arrType = llvm::ArrayType::get(llvm::Type::getInt32Ty(context), size);
        auto alloca = builder.CreateAlloca(arrType, nullptr, "array_tmp");

        for(int i = 0; i < size; i++)
        {
            auto val = generateExpr(arrLit->elements[i].get());
            auto ptr = builder.CreateGEP(arrType, alloca, {builder.getInt32(0), builder.getInt32(i)});
            builder.CreateStore(val, ptr);
        }
        return builder.CreateBitCast(alloca, llvm::PointerType::get(llvm::Type::getInt32Ty(context), 0));
    }

    // Tensor Literal: [[1, 2], [3, 4]]
    if(auto tensorLit = dynamic_cast<TensorLiteralExpr*>(expr)) {
        int rows = tensorLit->rows.size();
        int cols = rows > 0 ? tensorLit->rows[0].size() : 0;

        auto tensorPtr = builder.CreateCall(aiMatrixFunc, {
            builder.getInt32(rows),
            builder.getInt32(cols)
        }, "tensor_ptr");

        auto aiSetValueFunc = module->getFunction("ai_set_value");
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < (int)tensorLit->rows[i].size(); ++j) {
                llvm::Value* val = generateExpr(tensorLit->rows[i][j].get());
                
                if (val->getType()->isIntegerTy()) {
                    val = builder.CreateSIToFP(val, llvm::Type::getFloatTy(context));
                } else if (val->getType()->isDoubleTy()) {
                    val = builder.CreateFPTrunc(val, llvm::Type::getFloatTy(context));
                }

                builder.CreateCall(aiSetValueFunc, {
                    tensorPtr, 
                    builder.getInt32(i), 
                    builder.getInt32(j), 
                    val
                });
            }
        }
        return tensorPtr;
    }

    // Binary Expression (Arithmetic & Comparisons)
    if(auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        auto L = generateExpr(bin->left.get());
        auto R = generateExpr(bin->right.get());
        
        // Tensor Matmul
        if (bin->left->inferredType->isTensor() && bin->right->inferredType->isTensor()) {
            if (bin->op == "*") return builder.CreateCall(aiMatmulFunc, {L, R}, "matmul_tmp");
        }

        bool isFP = L->getType()->isDoubleTy() || R->getType()->isDoubleTy();

        if (bin->op == "+") return isFP ? builder.CreateFAdd(L, R) : builder.CreateAdd(L, R);
        if (bin->op == "-") return isFP ? builder.CreateFSub(L, R) : builder.CreateSub(L, R);
        if (bin->op == "*") return isFP ? builder.CreateFMul(L, R) : builder.CreateMul(L, R);
        if (bin->op == "/") return isFP ? builder.CreateFDiv(L, R) : builder.CreateSDiv(L, R);
        if (bin->op == "<") return isFP ? builder.CreateFCmpOLT(L, R) : builder.CreateICmpSLT(L, R);
        if (bin->op == ">") return isFP ? builder.CreateFCmpOGT(L, R) : builder.CreateICmpSGT(L, R);
        if (bin->op == "==") return isFP ? builder.CreateFCmpOEQ(L, R) : builder.CreateICmpEQ(L, R);
    }

    // Variables
    if(auto var = dynamic_cast<VariableExpr*>(expr))
    {
        auto ptr = namedValues[var->name];
        return builder.CreateLoad(
            getLLVMType(var->inferredType),
            ptr
        );
    }

    std::cerr << "Unknown expression in CodeGen\n";
    exit(1);

}