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

// Constructor initializes the reference to our LLVM state
CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

void CodeGen::generate(Program& program) {
    // Create the main function: i32 main()
    llvm::FunctionType* mainType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*llvm.context), false
    );

    llvm::Function* mainFunc = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", llvm.module.get()
    );

    // Create entry basic block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*llvm.context, "entry", mainFunc);
    llvm.builder->SetInsertPoint(entry);

    // Setup printf function for output
    std::vector<llvm::Type*> printfArgs = { llvm::PointerType::get(*llvm.context, 0) };
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*llvm.context), printfArgs, true
    );
    llvm::FunctionCallee printfFunc = llvm.module->getOrInsertFunction("printf", printfType);

    // Process every statement in the program
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

    // Return 0 from main
    llvm.builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*llvm.context), 0));
    
    // Validate the generated IR
    llvm::verifyFunction(*mainFunc);
}

void CodeGen::genPrintStmt(PrintStmt& stmt, llvm::FunctionCallee& printfFunc) {
    // Check if printing a variable that might be a matrix
    if (auto* vref = dynamic_cast<VarRef*>(stmt.expr.get())) {
        auto it = namedValues.find(vref->name);
        if (it != namedValues.end()) {
            llvm::Value* valPtr = it->second;
            
            if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(valPtr)) {
                llvm::Type* allocTy = alloca->getAllocatedType();

                // Handle Matrix/Array Printing
                if (allocTy->isArrayTy()) {
                    uint64_t numElems = allocTy->getArrayNumElements();
                    
                    llvm::Value* formatOpen = llvm.builder->CreateGlobalStringPtr("[ ");
                    llvm::Value* formatElem = llvm.builder->CreateGlobalStringPtr("%d ");
                    llvm::Value* formatClose = llvm.builder->CreateGlobalStringPtr("]\n");

                    llvm.builder->CreateCall(printfFunc, { formatOpen });

                    for (uint64_t i = 0; i < numElems; ++i) {
                        std::vector<llvm::Value*> indices = { 
                            llvm.builder->getInt32(0), 
                            llvm.builder->getInt32(i) 
                        };
                        
                        // Opaque pointer safe GEP
                        llvm::Value* elementPtr = llvm.builder->CreateInBoundsGEP(allocTy, alloca, indices);
                        llvm::Value* loadedVal = llvm.builder->CreateLoad(llvm::Type::getInt32Ty(*llvm.context), elementPtr);
                        
                        llvm.builder->CreateCall(printfFunc, { formatElem, loadedVal });
                    }

                    llvm.builder->CreateCall(printfFunc, { formatClose });
                    return; 
                }
            }
        }
    }

    // Default scalar printing (Integers and Strings)
    llvm::Value* value = genExpression(*stmt.expr);
    if (!value) return;

    llvm::Type* type = value->getType();
    if (type->isIntegerTy(32)) {
        llvm::Value* format = llvm.builder->CreateGlobalStringPtr("%d\n");
        llvm.builder->CreateCall(printfFunc, { format, value });
    } else if (type->isPointerTy()) {
        llvm::Value* format = llvm.builder->CreateGlobalStringPtr("%s\n");
        llvm.builder->CreateCall(printfFunc, { format, value });
    }
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
    llvm::AllocaInst* storage = llvm.builder->CreateAlloca(arrType, nullptr, decl.name);
    namedValues[decl.name] = storage;

    for (size_t i = 0; i < decl.elements.size(); ++i) {
        std::vector<llvm::Value*> indices = { 
            llvm.builder->getInt32(0), 
            llvm.builder->getInt32(i) 
        };
        llvm::Value* ptr = llvm.builder->CreateInBoundsGEP(arrType, storage, indices);
        llvm::Value* val = genExpression(*decl.elements[i]);
        llvm.builder->CreateStore(val, ptr);
    }
}

llvm::Value* CodeGen::genExpression(Expr& expr) {
    // Handle Integers (supports negative values via APInt)
    if (auto* i = dynamic_cast<IntegerLiteral*>(&expr)) {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*llvm.context), i->value, true);
    }
    
    // Handle Strings
    if (auto* s = dynamic_cast<StringLiteral*>(&expr)) {
        return llvm.builder->CreateGlobalStringPtr(s->value);
    }
    
    // Handle Variables (Variable references)
    if (auto* v = dynamic_cast<VarRef*>(&expr)) {
        auto it = namedValues.find(v->name);
        if (it == namedValues.end()) throw std::runtime_error("Undefined variable: " + v->name);

        llvm::Value* val = it->second;
        if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(val)) {
            llvm::Type* allocTy = alloca->getAllocatedType();
            
            // If it's a matrix, return the pointer to the array
            if (allocTy->isArrayTy()) return alloca; 
            
            // If it's a scalar (int), load the value from memory
            return llvm.builder->CreateLoad(allocTy, alloca, v->name);
        }
        return val;
    }
    
    // Default fallback
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*llvm.context), 0);
}

llvm::Type* CodeGen::toLLVMType(const TypeSpec& type) {
    // Currently mapping everything to i32 for simplicity
    return llvm::Type::getInt32Ty(*llvm.context);
}
