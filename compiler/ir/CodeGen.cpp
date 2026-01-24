#include "CodeGen.h"
#include "LLVMContext.h" // Ensure this file defines "struct LLVMState"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <stdexcept>

CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

void CodeGen::generate(Program& program) {
    for (auto& stmt : program.statements) {
        if (auto* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
            genVarDecl(*varDecl);
        }
    }
}

void CodeGen::genVarDecl(VarDecl& decl) {
    llvm::Type* type = toLLVMType(decl.type);
    // Future implementation: Allocate and store values
}

llvm::Type* CodeGen::toLLVMType(const TypeSpec& type) {
    // We dereference llvm.context assuming it's a pointer/unique_ptr inside LLVMState
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
