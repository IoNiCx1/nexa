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

    // printf
    auto printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(context),
        { llvm::PointerType::get(context, 0) },
        true   // ← FIX 1: was passing a raw Type* as isVarArg — must be a {params} list + bool
    );
    printfFunc = llvm::Function::Create(
        printfType, llvm::Function::ExternalLinkage, "printf", module.get()
    );

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
    }

    if (type->isInt())    return llvm::Type::getInt32Ty(context);
    if (type->isDouble()) return llvm::Type::getDoubleTy(context);
    if (type->isBool())   return llvm::Type::getInt1Ty(context);
    if (type->isString()) return llvm::PointerType::get(context, 0);
    if (type->isTensor()) return llvm::PointerType::get(context, 0);
    if (type->isArray())  return llvm::PointerType::get(llvm::Type::getInt32Ty(context), 0);

    return llvm::Type::getVoidTy(context);
}

// =============================
// Program Generation
// =============================

void CodeGen::generate(Program& program)
{
    // Forward-declare user functions first
    for (auto& stmt : program.statements)
        if (dynamic_cast<FunctionDecl*>(stmt.get()))
            generateStmt(stmt.get());

    // Build main()
    auto mainType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    auto mainFunc = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module.get()
    );
    auto entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    for (auto& stmt : program.statements)
        if (!dynamic_cast<FunctionDecl*>(stmt.get()))
            generateStmt(stmt.get());

    // Only emit ret if the block has no terminator yet
    // FIX 3: a return statement inside main would have already added a terminator,
    // emitting a second one causes a verifier crash / UB.
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));

    std::string errStr;
    llvm::raw_string_ostream errStream(errStr);
    if (llvm::verifyFunction(*mainFunc, &errStream)) {
        errStream.flush();
        std::cerr << "[CodeGen] main() verification failed:\n" << errStr << "\n";
        mainFunc->print(llvm::errs());
    }
}

// =============================
// Statement Generation
// =============================

void CodeGen::generateStmt(Stmt* stmt)
{
    if (!stmt) return;   // FIX 4: never deref a null stmt

    // ── Function Declaration ──────────────────────────────────
    if (auto fn = dynamic_cast<FunctionDecl*>(stmt))
    {
        std::vector<llvm::Type*> paramTypes;
        for (auto& p : fn->params)
            paramTypes.push_back(getLLVMType(p.second));

        auto funcType = llvm::FunctionType::get(getLLVMType(fn->returnType), paramTypes, false);
        auto function = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, fn->name, module.get()
        );

        auto block = llvm::BasicBlock::Create(context, "entry", function);
        auto* oldInsert = builder.GetInsertBlock();
        builder.SetInsertPoint(block);

        auto oldValues = namedValues;
        namedValues.clear();

        int idx = 0;
        for (auto& arg : function->args()) {
            auto paramName = fn->params[idx++].first;
            auto alloca = builder.CreateAlloca(arg.getType(), nullptr, paramName);
            builder.CreateStore(&arg, alloca);
            namedValues[paramName] = alloca;
        }

        for (auto& s : fn->body)
            generateStmt(s.get());

        if (!builder.GetInsertBlock()->getTerminator()) {
            if (fn->returnType && fn->returnType->isDouble())
                builder.CreateRet(llvm::ConstantFP::get(context, llvm::APFloat(0.0)));
            else if (fn->returnType && fn->returnType->isInt())
                builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
            else
                builder.CreateRetVoid();
        }

        namedValues = oldValues;
        if (oldInsert) builder.SetInsertPoint(oldInsert);
        return;
    }

    // ── Return ────────────────────────────────────────────────
    if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
        auto val = generateExpr(ret->value.get());
        if (val) builder.CreateRet(val);
        else     builder.CreateRetVoid();
        return;
    }

    // ── Variable Declaration ──────────────────────────────────
    if (auto var = dynamic_cast<VarDeclStmt*>(stmt)) {
        auto initVal = generateExpr(var->initializer.get());
        if (!initVal) {
            std::cerr << "[CodeGen] ERROR: initializer for '" << var->name << "' produced null\n";
            return;
        }
        auto alloca = builder.CreateAlloca(getLLVMType(var->declaredType), nullptr, var->name);
        builder.CreateStore(initVal, alloca);
        namedValues[var->name] = alloca;
        return;
    }

    // ── Assignment ────────────────────────────────────────────
    if (auto assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        auto value = generateExpr(assign->value.get());
        if (!value) { std::cerr << "[CodeGen] ERROR: assignment RHS is null\n"; return; }

        if (auto var = dynamic_cast<VariableExpr*>(assign->target.get())) {
            auto it = namedValues.find(var->name);
            if (it == namedValues.end()) {
                std::cerr << "[CodeGen] ERROR: assignment to undeclared variable '" << var->name << "'\n";
                return;
            }
            builder.CreateStore(value, it->second);
        }
        return;
    }

    // ── Print ─────────────────────────────────────────────────
    if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
        auto val = generateExpr(print->expression.get());
        if (!val) { std::cerr << "[CodeGen] ERROR: print expression is null\n"; return; }

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
    }

    // ── Loop ──────────────────────────────────────────────────
    if (auto loop = dynamic_cast<LoopStmt*>(stmt)) {
        auto countVal = generateExpr(loop->count.get());
        if (!countVal) { std::cerr << "[CodeGen] ERROR: loop count is null\n"; return; }

        auto* function = builder.GetInsertBlock()->getParent();

        // Save any existing binding so nested loops with the same iterator
        // name correctly restore the outer variable when the inner loop exits.
        llvm::Value* prevIterVal = nullptr;
        {
            auto prevIt = namedValues.find(loop->iterator);
            if (prevIt != namedValues.end())
                prevIterVal = prevIt->second;
        }

        auto loopVar = builder.CreateAlloca(llvm::Type::getInt32Ty(context), nullptr, loop->iterator);
        builder.CreateStore(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0), loopVar);
        namedValues[loop->iterator] = loopVar;

        auto condBlock = llvm::BasicBlock::Create(context, "loop_cond", function);
        auto bodyBlock = llvm::BasicBlock::Create(context, "loop_body", function);
        auto endBlock  = llvm::BasicBlock::Create(context, "loop_end",  function);

        builder.CreateBr(condBlock);
        builder.SetInsertPoint(condBlock);

        auto current = builder.CreateLoad(llvm::Type::getInt32Ty(context), loopVar, "loop_i");
        auto cond    = builder.CreateICmpSLT(current, countVal, "loop_cond");
        builder.CreateCondBr(cond, bodyBlock, endBlock);

        builder.SetInsertPoint(bodyBlock);
        for (auto& s : loop->body)
            generateStmt(s.get());

        // FIX 6: only emit the back-edge branch if the body didn't already terminate
        // (e.g. a return inside a loop would leave a terminator)
        if (!builder.GetInsertBlock()->getTerminator()) {
            auto next = builder.CreateAdd(
                builder.CreateLoad(llvm::Type::getInt32Ty(context), loopVar),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1)
            );
            builder.CreateStore(next, loopVar);
            builder.CreateBr(condBlock);
        }

        builder.SetInsertPoint(endBlock);

        // Restore previous binding (or remove if there was none)
        if (prevIterVal)
            namedValues[loop->iterator] = prevIterVal;
        else
            namedValues.erase(loop->iterator);
        return;
    }

    // ── If Statement ──────────────────────────────────────────
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        auto condVal = generateExpr(ifStmt->condition.get());
        if (!condVal) { std::cerr << "[CodeGen] ERROR: if condition is null\n"; return; }

        // Ensure condVal is i1
        if (!condVal->getType()->isIntegerTy(1))
            condVal = builder.CreateICmpNE(
                condVal,
                llvm::ConstantInt::get(condVal->getType(), 0),
                "bool_cond"
            );

        auto* function  = builder.GetInsertBlock()->getParent();
        auto thenBlock  = llvm::BasicBlock::Create(context, "then",   function);
        auto elseBlock  = llvm::BasicBlock::Create(context, "else",   function);
        auto mergeBlock = llvm::BasicBlock::Create(context, "ifcont", function);

        builder.CreateCondBr(condVal, thenBlock, elseBlock);

        // then
        builder.SetInsertPoint(thenBlock);
        for (auto& s : ifStmt->thenBranch)
            generateStmt(s.get());
        if (!builder.GetInsertBlock()->getTerminator())
            builder.CreateBr(mergeBlock);

        // else
        builder.SetInsertPoint(elseBlock);
        for (auto& s : ifStmt->elseBranch)
            generateStmt(s.get());
        if (!builder.GetInsertBlock()->getTerminator())
            builder.CreateBr(mergeBlock);

        builder.SetInsertPoint(mergeBlock);
        return;
    }

    std::cerr << "[CodeGen] WARNING: unhandled statement type: " << typeid(*stmt).name() << "\n";
}

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
        auto* fn = module->getFunction(call->callee);
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

    std::cerr << "[CodeGen] ERROR: unhandled expression type: " << typeid(*expr).name() << "\n";
    return nullptr;
}
