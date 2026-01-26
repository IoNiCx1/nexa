#include "CodeGen.h"
#include "LLVMContext.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <stdexcept>

CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

void CodeGen::generate(Program& program) {
    // 1. Create the 'main' function to return i32 (standard for exit codes)
    llvm::FunctionType* funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*llvm.context), false);
    llvm::Function* mainFunc = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, "main", llvm.module.get()
    );
    
    // 2. Create an 'entry' block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*llvm.context, "entry", mainFunc);
    llvm.builder->SetInsertPoint(entry);
    
    // NEW: Declare printf function: int printf(char*, ...)
    std::vector<llvm::Type*> printfArgs;
    // FIX: Use PointerType::get for newer LLVM versions
    printfArgs.push_back(llvm::PointerType::get(llvm::Type::getInt8Ty(*llvm.context), 0));
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*llvm.context), 
        printfArgs, 
        true  // varargs
    );
    llvm::FunctionCallee printfFunc = llvm.module->getOrInsertFunction("printf", printfType);
    
    // Default exit value if no variables are declared
    llvm::Value* lastValue = llvm::ConstantInt::get(*llvm.context, llvm::APInt(32, 0));
    
    // 3. Loop through statements and generate code
    for (auto& stmt : program.statements) {
        // NEW: Handle print statements
        if (auto* printStmt = dynamic_cast<PrintStmt*>(stmt.get())) {
            genPrintStmt(*printStmt, printfFunc);
        }
        // Handle variable declarations
        else if (auto* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
            genVarDecl(*varDecl);
            
            // For testing: Capture the value of the last declared variable to return it
            if (varDecl->initializer) {
                lastValue = genExpression(*varDecl->initializer);
            }
        }
    }
    
    // 4. Finish the function with a 'ret i32' instead of 'ret void'
    llvm.builder->CreateRet(lastValue);
    
    // Optional: Verify the generated code is valid
    llvm::verifyFunction(*mainFunc);
}

// NEW: Generate code for print statement
void CodeGen::genPrintStmt(PrintStmt& stmt, llvm::FunctionCallee& printfFunc) {
    llvm::Value* val = genExpression(*stmt.expression);
    
    if (!val) {
        throw std::runtime_error("Failed to generate expression for print statement");
    }
    
    llvm::Value* formatStr = nullptr;
    
    // Determine the correct format string based on the value type
    llvm::Type* valType = val->getType();
    
    if (valType->isIntegerTy(32)) {
        // Integer: use "%d\n"
        formatStr = llvm.builder->CreateGlobalStringPtr("%d\n");
    }
    else if (valType->isFloatTy()) {
        // Float: use "%f\n" (printf needs double, so we need to extend)
        val = llvm.builder->CreateFPExt(val, llvm::Type::getDoubleTy(*llvm.context));
        formatStr = llvm.builder->CreateGlobalStringPtr("%f\n");
    }
    else if (valType->isIntegerTy(8)) {
        // Char: use "%c\n"
        formatStr = llvm.builder->CreateGlobalStringPtr("%c\n");
    }
    else if (valType->isIntegerTy(1)) {
        // Bool: convert to int and use "%d\n"
        val = llvm.builder->CreateZExt(val, llvm::Type::getInt32Ty(*llvm.context));
        formatStr = llvm.builder->CreateGlobalStringPtr("%d\n");
    }
    else if (valType->isPointerTy()) {
        // String: use "%s\n"
        formatStr = llvm.builder->CreateGlobalStringPtr("%s\n");
    }
    else {
        throw std::runtime_error("Unsupported type for print statement");
    }
    
    // Create the printf call
    std::vector<llvm::Value*> args = { formatStr, val };
    llvm.builder->CreateCall(printfFunc, args);
}

void CodeGen::genVarDecl(VarDecl& decl) {
    llvm::Type* type = toLLVMType(decl.type);
    
    // Create 'alloca' on the stack
    llvm::Value* alloca = llvm.builder->CreateAlloca(type, nullptr, decl.name);
    
    // Store the initial value if it exists
    if (decl.initializer) {
        llvm::Value* initVal = genExpression(*decl.initializer);
        if (initVal) {
            llvm.builder->CreateStore(initVal, alloca);
        }
    }
    
    // Store the variable in the symbol table for later reference
    namedValues[decl.name] = alloca;
}

llvm::Type* CodeGen::toLLVMType(const TypeSpec& type) {
    switch (type.kind) {
        case TypeKind::Int:   return llvm::Type::getInt32Ty(*llvm.context);
        case TypeKind::Float: return llvm::Type::getFloatTy(*llvm.context);
        case TypeKind::Void:  return llvm::Type::getVoidTy(*llvm.context);
        default: throw std::runtime_error("Unknown type for LLVM lowering");
    }
}

llvm::Value* CodeGen::genExpression(Expr& expr) {
    // Integer literals
    if (auto* lit = dynamic_cast<IntegerLiteral*>(&expr)) {
        return llvm::ConstantInt::get(*llvm.context, llvm::APInt(32, lit->value, true));
    }
    // Float literals
    else if (auto* lit = dynamic_cast<FloatLiteral*>(&expr)) {
        return llvm::ConstantFP::get(*llvm.context, llvm::APFloat(lit->value));
    }
    // String literals
    else if (auto* lit = dynamic_cast<StringLiteral*>(&expr)) {
        return llvm.builder->CreateGlobalStringPtr(lit->value);
    }
    // Char literals
    else if (auto* lit = dynamic_cast<CharLiteral*>(&expr)) {
        return llvm::ConstantInt::get(*llvm.context, llvm::APInt(8, lit->value, false));
    }
    // Bool literals
    else if (auto* lit = dynamic_cast<BoolLiteral*>(&expr)) {
        return llvm::ConstantInt::get(*llvm.context, llvm::APInt(1, lit->value ? 1 : 0, false));
    }
    // Variable references
    // Variable references
else if (auto* varRef = dynamic_cast<VarRef*>(&expr)) {
    auto it = namedValues.find(varRef->name);
    if (it == namedValues.end()) {
        throw std::runtime_error("Undefined variable: " + varRef->name);
    }
    // Load the value from the stack
    llvm::AllocaInst* alloca = llvm::cast<llvm::AllocaInst>(it->second);
    return llvm.builder->CreateLoad(
        alloca->getAllocatedType(),
        it->second,
        varRef->name
    );
}
    
    return nullptr; 
}
