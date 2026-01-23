#include "CodeGen.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>

CodeGen::CodeGen(LLVMState& state) : llvm(state) {}

llvm::Type* CodeGen::toLLVMType(const TypeSpec& type) {
    using namespace llvm;

    switch (type.kind) {
        case TypeKind::I32:  return Type::getInt32Ty(llvm.context);
        case TypeKind::I64:  return Type::getInt64Ty(llvm.context);
        case TypeKind::F32:  return Type::getFloatTy(llvm.context);
        case TypeKind::F64:  return Type::getDoubleTy(llvm.context);
        case TypeKind::CHAR: return Type::getInt8Ty(llvm.context);
        case TypeKind::BOOL: return Type::getInt1Ty(llvm.context);
        case TypeKind::STRING:
            return Type::getInt8PtrTy(llvm.context);
    }
    return nullptr;
}

void CodeGen::generate(Program& program) {
    using namespace llvm;

    // int main()
    FunctionType* mainType =
        FunctionType::get(Type::getInt32Ty(llvm.context), false);

    Function* mainFunc =
        Function::Create(mainType, Function::ExternalLinkage,
                         "main", llvm.module.get());

    BasicBlock* entry =
        BasicBlock::Create(llvm.context, "entry", mainFunc);

    llvm.builder.SetInsertPoint(entry);

    for (auto& stmt : program.statements) {
        genDeclaration(static_cast<Declaration&>(*stmt));
    }

    llvm.builder.CreateRet(
        ConstantInt::get(Type::getInt32Ty(llvm.context), 0));

    verifyFunction(*mainFunc);
}

void CodeGen::genDeclaration(Declaration& decl) {
    llvm::Type* llvmType = toLLVMType(decl.type);

    for (size_t i = 0; i < decl.names.size(); ++i) {
        const std::string& name = decl.names[i];

        llvm::AllocaInst* alloc =
            llvm.builder.CreateAlloca(llvmType, nullptr, name);

        llvm::Value* value = genExpression(*decl.values[i]);
        llvm.builder.CreateStore(value, alloc);

        namedValues[name] = alloc;
    }
}

llvm::Value* CodeGen::genExpression(Expr& expr) {
    using namespace llvm;

    if (auto* i = dynamic_cast<IntLiteral*>(&expr)) {
        return ConstantInt::get(
            Type::getInt32Ty(llvm.context), i->value);
    }

    if (auto* f = dynamic_cast<FloatLiteral*>(&expr)) {
        return ConstantFP::get(
            Type::getDoubleTy(llvm.context), f->value);
    }

    if (auto* b = dynamic_cast<BoolLiteral*>(&expr)) {
        return ConstantInt::get(
            Type::getInt1Ty(llvm.context), b->value);
    }

    if (auto* c = dynamic_cast<CharLiteral*>(&expr)) {
        return ConstantInt::get(
            Type::getInt8Ty(llvm.context), c->value);
    }

    if (auto* s = dynamic_cast<StringLiteral*>(&expr)) {
        return llvm.builder.CreateGlobalStringPtr(s->value);
    }

    if (auto* v = dynamic_cast<VariableRef*>(&expr)) {
        return llvm.builder.CreateLoad(
            namedValues[v->name]->getType()->getPointerElementType(),
            namedValues[v->name]);
    }

    return nullptr;
}
