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
<<<<<<< Updated upstream
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
=======

    // ai_create_matrix
    auto matrixType = llvm::FunctionType::get(
        llvm::PointerType::get(context, 0),
        { llvm::Type::getInt32Ty(context), llvm::Type::getInt32Ty(context) },
        false
    );
    aiMatrixFunc = llvm::Function::Create(
        matrixType, llvm::Function::ExternalLinkage, "ai_create_matrix", module.get()
    );

    // ai_set_value
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
        setValType, llvm::Function::ExternalLinkage, "ai_set_value", module.get()
    );

    // ai_matmul
    auto matmulType = llvm::FunctionType::get(
        llvm::PointerType::get(context, 0),
        { llvm::PointerType::get(context, 0), llvm::PointerType::get(context, 0) },
        false
    );
    aiMatmulFunc = llvm::Function::Create(
        matmulType, llvm::Function::ExternalLinkage, "ai_matmul", module.get()
    );

    // ai_print
    auto printTensorType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(context),
        { llvm::PointerType::get(context, 0) },
        false
    );

    // ai_zeros(int rows, int cols) -> Tensor*
    auto zerosType = llvm::FunctionType::get(
            llvm::PointerType::get(context, 0),
            { llvm::Type::getInt32Ty(context), llvm::Type::getInt32Ty(context) },
            false
    );
    llvm::Function::Create(zerosType, llvm::Function::ExternalLinkage, "ai_zeros", module.get());

    // ai_sum(Tensor*) -> float
    auto sumType = llvm::FunctionType::get(
            llvm::Type::getFloatTy(context),
            { llvm::PointerType::get(context, 0) },
            false
    );
    llvm::Function::Create(sumType, llvm::Function::ExternalLinkage, "ai_sum", module.get());

    // ai_reshape(Tensor*, int r, int c) -> Tensor*
    auto reshapeType = llvm::FunctionType::get(
            llvm::PointerType::get(context, 0),
            { llvm::PointerType::get(context, 0), llvm::Type::getInt32Ty(context), 
                llvm::Type::getInt32Ty(context) },
            false
    );
    llvm::Function::Create(reshapeType, llvm::Function::ExternalLinkage, "ai_reshape", module.get());

    auto tensorToFloatType = llvm::FunctionType::get(
        llvm::Type::getFloatTy(context),
        { llvm::PointerType::get(context, 0) },
        false
    );
    llvm::Function::Create(tensorToFloatType, llvm::Function::ExternalLinkage, "ai_mean", module.get());
    llvm::Function::Create(tensorToFloatType, llvm::Function::ExternalLinkage, "ai_max", module.get());
    llvm::Function::Create(tensorToFloatType, llvm::Function::ExternalLinkage, "ai_min", module.get());

    auto shapeType = llvm::FunctionType::get(
        llvm::PointerType::get(context, 0),
        { llvm::PointerType::get(context, 0) },
        false
    );
    llvm::Function::Create(shapeType, llvm::Function::ExternalLinkage, "ai_shape", module.get());

    aiPrintTensorFunc = llvm::Function::Create(
        printTensorType, llvm::Function::ExternalLinkage, "ai_print", module.get()
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
    // FIX 2: null type guard — SemanticAnalyzer may leave inferredType null
    if (!type) {
        std::cerr << "[CodeGen] WARNING: getLLVMType called with null type, defaulting to i32\n";
        return llvm::Type::getInt32Ty(context);
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
    // Bool literals
    else if (auto* lit = dynamic_cast<BoolLiteral*>(&expr)) {
        return llvm::ConstantInt::get(*llvm.context, llvm::APInt(1, lit->value ? 1 : 0, false));
=======

    // ── Print ─────────────────────────────────────────────────
    if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
        auto val = generateExpr(print->expression.get());
        if (!val) { std::cerr << "[CodeGen] ERROR: print expression is null\n"; return; }
        if (val->getType()->isVoidTy()) return;

        auto* type = print->expression->inferredType;

        // Derive format from actual IR type when inferredType is missing
        const char* fmt = nullptr;
        bool needZExt   = false;

        if (type) {
            if      (type->isTensor()) { builder.CreateCall(aiPrintTensorFunc, {val}); return; }
            else if (type->isDouble()) fmt = "%.6g\n";
            else if (type->isString()) fmt = "%s\n";
            else if (type->isBool())   { fmt = "%d\n"; needZExt = true; }
            else                       fmt = "%d\n";
        } else {
            // Fallback: inspect the LLVM IR type directly
            auto* irTy = val->getType();
            if      (irTy->isDoubleTy())  fmt = "%.6g\n";
            else if (irTy->isPointerTy()) fmt = "%s\n";
            else if (irTy->isIntegerTy(1)){ fmt = "%d\n"; needZExt = true; }
            else                          fmt = "%d\n";
        }

        if (needZExt)
            val = builder.CreateZExt(val, llvm::Type::getInt32Ty(context));

        auto fmtVal = builder.CreateGlobalStringPtr(fmt);
        builder.CreateCall(printfFunc, {fmtVal, val});
        return;
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
=======

// =============================
// Expression Generation
// =============================

llvm::Value* CodeGen::generateExpr(Expr* expr)
{
    if (!expr) return nullptr;   // FIX 7: every caller must null-check the return

    // ── Literals ──────────────────────────────
    if (auto intLit = dynamic_cast<IntegerLiteral*>(expr))
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), intLit->value, true);

    if (auto dblLit = dynamic_cast<DoubleLiteral*>(expr))
        return llvm::ConstantFP::get(context, llvm::APFloat(dblLit->value));

    if (auto boolLit = dynamic_cast<BoolLiteral*>(expr))
        return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), boolLit->value ? 1 : 0);

    if (auto strLit = dynamic_cast<StringLiteral*>(expr))
        return builder.CreateGlobalStringPtr(strLit->value);

    // ── Grouping ──────────────────────────────
    if (auto group = dynamic_cast<GroupingExpr*>(expr))
        return generateExpr(group->expression.get());

    // ── Array Indexing ─────────────────────────
    if (auto idxExpr = dynamic_cast<IndexExpr*>(expr)) {
        auto arr = generateExpr(idxExpr->array.get());
        auto idx = generateExpr(idxExpr->index.get());
        if (!arr || !idx) return nullptr;


        // arr must be a pointer for GEP. If the array variable was loaded
        // as an integer (type mismatch), inttoptr it so we get valid IR.
        // In correct operation arr is already ptr — this is a safety net.
        if (!arr->getType()->isPointerTy()) {
            std::cerr << "[CodeGen] WARNING: array base is not a pointer — casting via inttoptr\n";
            arr = builder.CreateIntToPtr(arr, llvm::PointerType::get(context, 0), "arr_ptr_cast");
        }

        auto ptr = builder.CreateGEP(
            llvm::Type::getInt32Ty(context), arr, idx, "elem_ptr"
        );
        return builder.CreateLoad(llvm::Type::getInt32Ty(context), ptr, "elem");
    }

    // ── Array Literal ─────────────────────────
    if (auto arrLit = dynamic_cast<ArrayLiteralExpr*>(expr)) {
        int size    = (int)arrLit->elements.size();
        auto arrTy  = llvm::ArrayType::get(llvm::Type::getInt32Ty(context), size);
        auto alloca = builder.CreateAlloca(arrTy, nullptr, "array_tmp");

        for (int i = 0; i < size; ++i) {
            auto val = generateExpr(arrLit->elements[i].get());
            if (!val) continue;
            auto ptr = builder.CreateGEP(arrTy, alloca,
                { builder.getInt32(0), builder.getInt32(i) }, "elem_ptr");
            builder.CreateStore(val, ptr);
        }
        // Decay [N x i32]* → i32*
        return builder.CreateGEP(arrTy, alloca,
            { builder.getInt32(0), builder.getInt32(0) }, "arr_ptr");
    }

    // ── Tensor Literal ────────────────────────
    if (auto tensorLit = dynamic_cast<TensorLiteralExpr*>(expr)) {
        int rows = (int)tensorLit->rows.size();
        int cols = rows > 0 ? (int)tensorLit->rows[0].size() : 0;

        auto tensorPtr = builder.CreateCall(
            aiMatrixFunc, { builder.getInt32(rows), builder.getInt32(cols) }, "tensor_ptr"
        );
        auto* aiSetValueFunc = module->getFunction("ai_set_value");

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < (int)tensorLit->rows[i].size(); ++j) {
                auto val = generateExpr(tensorLit->rows[i][j].get());
                if (!val) continue;
                if (val->getType()->isIntegerTy())
                    val = builder.CreateSIToFP(val, llvm::Type::getFloatTy(context));
                else if (val->getType()->isDoubleTy())
                    val = builder.CreateFPTrunc(val, llvm::Type::getFloatTy(context));
                builder.CreateCall(aiSetValueFunc,
                    { tensorPtr, builder.getInt32(i), builder.getInt32(j), val });
            }
        }
        return tensorPtr;
    }

    // ── Binary Expression ─────────────────────
    if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
        auto L = generateExpr(bin->left.get());
        auto R = generateExpr(bin->right.get());
        if (!L || !R) return nullptr;

        // Tensor matmul
        auto* lt = bin->left->inferredType;
        auto* rt = bin->right->inferredType;
        if (lt && rt && lt->isTensor() && rt->isTensor() && bin->op == "*")
            return builder.CreateCall(aiMatmulFunc, {L, R}, "matmul_tmp");

        // FIX 8: promote int→double when sides are mixed so FAdd/FSub don't
        // receive an i32 on one side and double on the other → verifier error
        bool lFP = L->getType()->isDoubleTy();
        bool rFP = R->getType()->isDoubleTy();
        if (lFP && !rFP)
            R = builder.CreateSIToFP(R, llvm::Type::getDoubleTy(context));
        else if (!lFP && rFP)
            L = builder.CreateSIToFP(L, llvm::Type::getDoubleTy(context));

        bool isFP = L->getType()->isDoubleTy();

        if (bin->op == "+")  return isFP ? builder.CreateFAdd(L, R, "fadd") : builder.CreateAdd (L, R, "iadd");
        if (bin->op == "-")  return isFP ? builder.CreateFSub(L, R, "fsub") : builder.CreateSub (L, R, "isub");
        if (bin->op == "*")  return isFP ? builder.CreateFMul(L, R, "fmul") : builder.CreateMul (L, R, "imul");
        if (bin->op == "/")  return isFP ? builder.CreateFDiv(L, R, "fdiv") : builder.CreateSDiv(L, R, "idiv");
        if (bin->op == "<")  return isFP ? builder.CreateFCmpOLT(L, R, "fcmp") : builder.CreateICmpSLT(L, R, "icmp");
        if (bin->op == ">")  return isFP ? builder.CreateFCmpOGT(L, R, "fcmp") : builder.CreateICmpSGT(L, R, "icmp");
        if (bin->op == "==") return isFP ? builder.CreateFCmpOEQ(L, R, "fcmp") : builder.CreateICmpEQ (L, R, "icmp");
        if (bin->op == "!=") return isFP ? builder.CreateFCmpONE(L, R, "fcmp") : builder.CreateICmpNE (L, R, "icmp");
        if (bin->op == "<=") return isFP ? builder.CreateFCmpOLE(L, R, "fcmp") : builder.CreateICmpSLE(L, R, "icmp");
        if (bin->op == ">=") return isFP ? builder.CreateFCmpOGE(L, R, "fcmp") : builder.CreateICmpSGE(L, R, "icmp");

        std::cerr << "[CodeGen] ERROR: unknown binary op '" << bin->op << "'\n";
        return nullptr;
    }

    // ── Variable Reference ────────────────────
    if (auto var = dynamic_cast<VariableExpr*>(expr)) {
        auto it = namedValues.find(var->name);
        if (it == namedValues.end() || !it->second) {
            std::cerr << "[CodeGen] ERROR: undefined variable '" << var->name << "'\n";
            return nullptr;
        }

        if (var->inferredType && var->inferredType->isStruct()) {
            return it->second;
        }

        // Arrays and strings are stored as ptr-to-ptr (alloca ptr).
        // Loading them must yield a ptr, not i32.
        // Use the alloca's allocated type to decide the load type so we
        // never mismatch the stored value's actual IR type.
        llvm::Type* loadTy = nullptr;
        if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(it->second)) {
            loadTy = allocaInst->getAllocatedType();
        } else {
            loadTy = var->inferredType
                ? getLLVMType(var->inferredType)
                : llvm::Type::getInt32Ty(context);
        }
        return builder.CreateLoad(loadTy, it->second, var->name);
    }

    // ── Function Call ─────────────────────────
    if (auto call = dynamic_cast<CallExpr*>(expr)) {
        std::string funcName = call->callee;

        // Translation layer: Map Nexa names to Runtime (ai_) names
        if      (funcName == "zeros")   funcName = "ai_zeros";
        else if (funcName == "ones")    funcName = "ai_ones";
        else if (funcName == "sum")     funcName = "ai_sum";
        else if (funcName == "mean")    funcName = "ai_mean";
        else if (funcName == "max")     funcName = "ai_max";
        else if (funcName == "min")     funcName = "ai_min";
        else if (funcName == "reshape") funcName = "ai_reshape";
        else if (funcName == "shape")   funcName = "ai_shape";

        auto* fn = module->getFunction(funcName); 
        if (!fn) {
            std::cerr << "[CodeGen] ERROR: unknown function '" << call->callee << "'\n";
            return nullptr;
        }

        std::vector<llvm::Value*> args;
        for (auto& arg : call->arguments) {
            auto val = generateExpr(arg.get());
            if (!val) return nullptr;
            args.push_back(val);
        }
        return builder.CreateCall(fn, args, "calltmp");
    }

    // ── Struct Literal ────────────────────────────────────────
    if (auto sl = dynamic_cast<StructLiteralExpr*>(expr)) {
        auto it = structTypes.find(sl->structName);
        if (it == structTypes.end()) {
            std::cerr << "[CodeGen] ERROR: unknown struct '" << sl->structName << "'\n";
            return nullptr;
        }
        auto* st     = it->second;
        auto* alloca = builder.CreateAlloca(st, nullptr, sl->structName + "_tmp");

        auto& fieldIdx = structFields[sl->structName];

        for (auto& f : sl->fields) {
            int idx = -1;
            for (auto& fi : fieldIdx)
                if (fi.first == f.first) { idx = fi.second; break; }
            if (idx == -1) {
                std::cerr << "[CodeGen] ERROR: unknown field '" << f.first << "'\n";
                continue;
            }
            auto val = generateExpr(f.second.get());
            if (!val) continue;
            auto ptr = builder.CreateStructGEP(st, alloca, idx, f.first + "_ptr");
            builder.CreateStore(val, ptr);
        }
        return alloca;
    }

    // ── Member Access ─────────────────────────────────────────
    if (auto ma = dynamic_cast<MemberAccessExpr*>(expr)) {
        auto obj = generateExpr(ma->object.get());
        if (!obj) return nullptr;

        std::string structName;
        if (ma->object->inferredType)
            structName = ma->object->inferredType->structName;

        auto sit = structTypes.find(structName);
        if (sit == structTypes.end()) {
            std::cerr << "[CodeGen] ERROR: cannot resolve struct type for member access\n";
            return nullptr;
        }

        auto* st       = sit->second;
        auto& fieldIdx = structFields[structName];

        int idx = -1;
        for (auto& fi : fieldIdx)
            if (fi.first == ma->field) { idx = fi.second; break; }
        if (idx == -1) {
            std::cerr << "[CodeGen] ERROR: unknown field '" << ma->field << "'\n";
            return nullptr;
        }

        auto ptr = builder.CreateStructGEP(st, obj, idx, ma->field + "_ptr");
        return builder.CreateLoad(st->getElementType(idx), ptr, ma->field);
    }

    //----- Constructor Call Expression ----------------------
    if (auto cc = dynamic_cast<ConstructorCallExpr*>(expr)) {
        auto* st = structTypes[cc->structName];
        if (!st) {
            std::cerr << "[Codegen] ERROR: unkown struct '" << cc->structName << "'\n";
            return nullptr;
        }

        //Allocate struct on stack
        auto* alloca = builder.CreateAlloca(st, nullptr, cc->structName + "_inst");

        //Call constructor function if it exists
        auto* ctorFunc = module->getFunction(cc->structName + "__ctor");
        if (ctorFunc) {
            std::vector<llvm::Value*> args;
            args.push_back(alloca);
            for (auto& arg :cc->arguments) {
                auto val =generateExpr(arg.get());
                if (!val) return nullptr;
                args.push_back(val);
            }
            builder.CreateCall(ctorFunc, args);
        }
        return alloca;
    }

    std::cerr<<"[CodeGen] ERROR: unhandled expression type: "<< typeid(*expr).name() << "\n";
    return nullptr;
}
>>>>>>> Stashed changes
