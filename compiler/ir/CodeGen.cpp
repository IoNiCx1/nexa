#include "CodeGen.h"
#include "LLVMContext.h"
#include "../ast/Ast.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>

#include <stdexcept>
#include <vector>
#include <map>

CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

void CodeGen::generate(Program& program) {
    llvm::FunctionType* mainType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*llvm.context), false
    );

    llvm::Function* mainFunc = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", llvm.module.get()
    );

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*llvm.context, "entry", mainFunc);
    llvm.builder->SetInsertPoint(entry);

    std::vector<llvm::Type*> printfArgs = { llvm::PointerType::get(*llvm.context, 0) };
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*llvm.context), printfArgs, true
    );
    llvm::FunctionCallee printfFunc = llvm.module->getOrInsertFunction("printf", printfType);

    for (auto& stmt : program.statements) {
        if (auto* p = dynamic_cast<PrintStmt*>(stmt.get())) {
            genPrintStmt(*p, printfFunc);
        }
        else if (auto* v = dynamic_cast<VarDecl*>(stmt.get())) {
            genVarDecl(*v);
        }
        else if (auto* a = dynamic_cast<ArrayDecl*>(stmt.get())) {
            genArrayDecl(*a);
        }
    }

    llvm.builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*llvm.context), 0));
    llvm::verifyFunction(*mainFunc);
}

void CodeGen::genPrintStmt(PrintStmt& stmt, llvm::FunctionCallee& printfFunc) {
    llvm::Value* value = genExpression(*stmt.expr);
    if (!value) return;

    llvm::Value* format = nullptr;
    llvm::Type* type = value->getType();

    if (type->isIntegerTy(32)) {
        format = llvm.builder->CreateGlobalStringPtr("%d\n");
    } else if (type->isPointerTy()) {
        format = llvm.builder->CreateGlobalStringPtr("%s\n");
    } else {
        format = llvm.builder->CreateGlobalStringPtr("[Nexa Object]\n");
    }

    llvm.builder->CreateCall(printfFunc, { format, value });
}

void CodeGen::genVarDecl(VarDecl& decl) {
    llvm::Type* type = toLLVMType(decl.type);
    llvm::AllocaInst* alloca = llvm.builder->CreateAlloca(type, nullptr, decl.name);

    if (decl.initializer) {
        llvm::Value* initVal = genExpression(*decl.initializer);
        llvm.builder->CreateStore(initVal, alloca);
    }
    namedValues[decl.name] = alloca;
}

void CodeGen::genArrayDecl(ArrayDecl& decl) {
    size_t size = decl.elements.size();
    if (size == 0) size = 1; 

    llvm::ArrayType* arrType = llvm::ArrayType::get(llvm::Type::getInt32Ty(*llvm.context), size);
    llvm::Value* storage;

    if (namedValues.count(decl.name)) {
        storage = namedValues[decl.name];
    } else {
        storage = llvm.builder->CreateAlloca(arrType, nullptr, decl.name);
        namedValues[decl.name] = storage;
    }

    for (size_t i = 0; i < decl.elements.size(); ++i) {
        std::vector<llvm::Value*> indices = { llvm.builder->getInt32(0), llvm.builder->getInt32(i) };
        llvm::Value* ptr = llvm.builder->CreateInBoundsGEP(arrType, storage, indices);
        llvm::Value* val = genExpression(*decl.elements[i]);
        llvm.builder->CreateStore(val, ptr);
    }
}

llvm::Value* CodeGen::genExpression(Expr& expr) {
    if (auto* i = dynamic_cast<IntegerLiteral*>(&expr)) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*llvm.context), i->value);
    }
    if (auto* s = dynamic_cast<StringLiteral*>(&expr)) {
        return llvm.builder->CreateGlobalStringPtr(s->value);
    }
    if (auto* v = dynamic_cast<VarRef*>(&expr)) {
        auto it = namedValues.find(v->name);
        if (it == namedValues.end()) throw std::runtime_error("Undefined variable: " + v->name);

        llvm::Value* val = it->second;
        if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(val)) {
            llvm::Type* allocTy = alloca->getAllocatedType();
            if (allocTy->isArrayTy()) return alloca;
            return llvm.builder->CreateLoad(allocTy, alloca, v->name);
        }
        return val;
    }
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*llvm.context), 0);
}

llvm::Type* CodeGen::toLLVMType(const TypeSpec& type) {
    return llvm::Type::getInt32Ty(*llvm.context);
}
