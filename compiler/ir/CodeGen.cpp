#include "CodeGen.h"
#include "LLVMContext.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
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

    // Default exit value if no variables are declared
    llvm::Value* lastValue = llvm::ConstantInt::get(*llvm.context, llvm::APInt(32, 0));

    // 3. Loop through statements and generate code
    for (auto& stmt : program.statements) {
        if (auto* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
            genVarDecl(*varDecl);
            
            // For testing: Capture the value of the last declared variable to return it
            if (varDecl->initializer) {
                lastValue = genExpression(*varDecl->initializer);
            }
        }
    }

    // 4. Finish the function with a 'ret i32' instead of 'ret void'
    // This allows 'echo $?' to show the actual value
    llvm.builder->CreateRet(lastValue);

    // Optional: Verify the generated code is valid
    llvm::verifyFunction(*mainFunc);
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
    if (auto* lit = dynamic_cast<IntegerLiteral*>(&expr)) {
        return llvm::ConstantInt::get(*llvm.context, llvm::APInt(32, lit->value, true));
    }
    return nullptr; 
}
