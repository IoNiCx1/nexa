#include "CodeGen.h"

#include <iostream>
#include <fstream>

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
        true
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

    // ai_mean / ai_max / ai_min (Tensor*) -> float
    auto tensorToFloatType = llvm::FunctionType::get(
        llvm::Type::getFloatTy(context),
        { llvm::PointerType::get(context, 0) },
        false
    );
    llvm::Function::Create(tensorToFloatType, llvm::Function::ExternalLinkage, "ai_mean",  module.get());
    llvm::Function::Create(tensorToFloatType, llvm::Function::ExternalLinkage, "ai_max",   module.get());
    llvm::Function::Create(tensorToFloatType, llvm::Function::ExternalLinkage, "ai_min",   module.get());

    // ai_shape(Tensor*) -> ptr
    auto shapeType = llvm::FunctionType::get(
        llvm::PointerType::get(context, 0),
        { llvm::PointerType::get(context, 0) },
        false
    );
    llvm::Function::Create(shapeType, llvm::Function::ExternalLinkage, "ai_shape", module.get());

    // ai_get_value(Tensor*, int r, int c) -> float
    auto getValType = llvm::FunctionType::get(
        llvm::Type::getFloatTy(context),
        { llvm::PointerType::get(context, 0),
          llvm::Type::getInt32Ty(context),
          llvm::Type::getInt32Ty(context) },
        false
    );
    llvm::Function::Create(getValType, llvm::Function::ExternalLinkage, "ai_get_value", module.get());

    // ── File runtime ──────────────────────────
    declareFileRuntime();
    declareCsvRuntime();
    declareMlRuntime();
}

// ── File runtime declarations ─────────────────────────────────────────────────

void CodeGen::declareFileRuntime() {
    auto* ptrTy  = llvm::PointerType::get(context, 0);
    auto* voidTy = llvm::Type::getVoidTy(context);
    auto* i32Ty  = llvm::Type::getInt32Ty(context);

    // char* nexa_file_read(char* path)
    module->getOrInsertFunction("nexa_file_read",
        llvm::FunctionType::get(ptrTy, {ptrTy}, false));

    // void nexa_file_write(char* path, char* content)
    module->getOrInsertFunction("nexa_file_write",
        llvm::FunctionType::get(voidTy, {ptrTy, ptrTy}, false));

    // void nexa_file_append(char* path, char* content)
    module->getOrInsertFunction("nexa_file_append",
        llvm::FunctionType::get(voidTy, {ptrTy, ptrTy}, false));

    // int nexa_file_exists(char* path)
    module->getOrInsertFunction("nexa_file_exists",
        llvm::FunctionType::get(i32Ty, {ptrTy}, false));
}

// ── CSV runtime declarations ──────────────────────────────────────────────────

void CodeGen::declareCsvRuntime() {
    auto* ptrTy  = llvm::PointerType::get(context, 0);
    auto* voidTy = llvm::Type::getVoidTy(context);
    auto* i32Ty  = llvm::Type::getInt32Ty(context);
    auto* f32Ty  = llvm::Type::getFloatTy(context);

    // void* csv_read(char* path, int skip_header)
    module->getOrInsertFunction("csv_read",
        llvm::FunctionType::get(ptrTy, {ptrTy, i32Ty}, false));

    // void csv_write(char* path, void* tensor)
    module->getOrInsertFunction("csv_write",
        llvm::FunctionType::get(voidTy, {ptrTy, ptrTy}, false));

    // int csv_rows(void* tensor)
    module->getOrInsertFunction("csv_rows",
        llvm::FunctionType::get(i32Ty, {ptrTy}, false));

    // int csv_cols(void* tensor)
    module->getOrInsertFunction("csv_cols",
        llvm::FunctionType::get(i32Ty, {ptrTy}, false));

    // float csv_get(void* tensor, int row, int col)
    module->getOrInsertFunction("csv_get",
        llvm::FunctionType::get(f32Ty, {ptrTy, i32Ty, i32Ty}, false));

    // void csv_set(void* tensor, int row, int col, float val)
    module->getOrInsertFunction("csv_set",
        llvm::FunctionType::get(voidTy, {ptrTy, i32Ty, i32Ty, f32Ty}, false));

    // void* csv_get_row(void* tensor, int row)
    module->getOrInsertFunction("csv_get_row",
        llvm::FunctionType::get(ptrTy, {ptrTy, i32Ty}, false));

    // void* csv_get_col(void* tensor, int col)
    module->getOrInsertFunction("csv_get_col",
        llvm::FunctionType::get(ptrTy, {ptrTy, i32Ty}, false));

    // void* csv_slice_cols(void* tensor, int col_start, int col_end)
    module->getOrInsertFunction("csv_slice_cols",
        llvm::FunctionType::get(ptrTy, {ptrTy, i32Ty, i32Ty}, false));
}

// ── ML runtime declarations ───────────────────────────────────────────────────

void CodeGen::declareMlRuntime() {
    auto* ptrTy  = llvm::PointerType::get(context, 0);
    auto* voidTy = llvm::Type::getVoidTy(context);
    auto* i32Ty  = llvm::Type::getInt32Ty(context);
    auto* f32Ty  = llvm::Type::getFloatTy(context);

    // void* ml_normalize(void* X)
    module->getOrInsertFunction("ml_normalize",
        llvm::FunctionType::get(ptrTy, {ptrTy}, false));

    // void* ml_shuffle(void* X)
    module->getOrInsertFunction("ml_shuffle",
        llvm::FunctionType::get(ptrTy, {ptrTy}, false));

    // void* ml_train_split(void* X, float ratio)
    module->getOrInsertFunction("ml_train_split",
        llvm::FunctionType::get(ptrTy, {ptrTy, f32Ty}, false));

    // void* ml_test_split(void* X, float ratio)
    module->getOrInsertFunction("ml_test_split",
        llvm::FunctionType::get(ptrTy, {ptrTy, f32Ty}, false));

    // void* ml_hstack(void* A, void* B)
    module->getOrInsertFunction("ml_hstack",
        llvm::FunctionType::get(ptrTy, {ptrTy, ptrTy}, false));

    // void* lore_create(int max_iter, float lr)
    module->getOrInsertFunction("lore_create",
        llvm::FunctionType::get(ptrTy, {i32Ty, f32Ty}, false));

    // void lore_fit(void* model, void* X, void* y)
    module->getOrInsertFunction("lore_fit",
        llvm::FunctionType::get(voidTy, {ptrTy, ptrTy, ptrTy}, false));

    // void* lore_predict(void* model, void* X)
    module->getOrInsertFunction("lore_predict",
        llvm::FunctionType::get(ptrTy, {ptrTy, ptrTy}, false));

    // void* lore_predict_proba(void* model, void* X)
    module->getOrInsertFunction("lore_predict_proba",
        llvm::FunctionType::get(ptrTy, {ptrTy, ptrTy}, false));

    // float ml_accuracy(void* pred, void* labels)
    module->getOrInsertFunction("ml_accuracy",
        llvm::FunctionType::get(f32Ty, {ptrTy, ptrTy}, false));

    // void* ml_confusion(void* pred, void* labels)
    module->getOrInsertFunction("ml_confusion",
        llvm::FunctionType::get(ptrTy, {ptrTy, ptrTy}, false));

    // void* lore_weights(void* model)
    module->getOrInsertFunction("lore_weights",
        llvm::FunctionType::get(ptrTy, {ptrTy}, false));

    // float lore_bias(void* model)
    module->getOrInsertFunction("lore_bias",
        llvm::FunctionType::get(f32Ty, {ptrTy}, false));
}

llvm::Module* CodeGen::getModule() {
    return module.get();
}

// =============================
// Type Mapping
// =============================

llvm::Type* CodeGen::getLLVMType(Type* type)
{
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
    if (type->isStruct()) return llvm::PointerType::get(context, 0);

    return llvm::Type::getVoidTy(context);
}

// =============================
// Program Generation
// =============================

void CodeGen::generate(Program& program)
{
    // Forward-declare user functions and structs first
    for (auto& stmt : program.statements)
        if (dynamic_cast<FunctionDecl*>(stmt.get()) ||
            dynamic_cast<StructDecl*>(stmt.get()))
            generateStmt(stmt.get());

    // Build main()
    auto mainType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    auto mainFunc = llvm::Function::Create(
        mainType, llvm::Function::ExternalLinkage, "main", module.get()
    );
    auto entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    for (auto& stmt : program.statements)
        if (!dynamic_cast<FunctionDecl*>(stmt.get()) &&
            !dynamic_cast<StructDecl*>(stmt.get()))
            generateStmt(stmt.get());

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
    if (!stmt) return;

    // ── ImpStmt: imp open.file() ──────────────
    if (dynamic_cast<ImpStmt*>(stmt)) {
        fileModuleImported = true;
        return;
    }

    // ── FileStmt: open.file(...) as statement ──
    if (auto fs = dynamic_cast<FileStmt*>(stmt)) {
        generateFileExpr(fs->expr.get());
        return;
    }

    // ── Struct Declaration ────────────────────
    if (auto sd = dynamic_cast<StructDecl*>(stmt)) {
        std::vector<llvm::Type*> fieldTypes;
        std::vector<std::pair<std::string, int>> fieldMap;
        int idx = 0;
        for (auto& [name, type] : sd->fields) {
            fieldTypes.push_back(getLLVMType(type));
            fieldMap.push_back({name, idx++});
        }
        auto* st = llvm::StructType::create(context, fieldTypes, sd->name);
        structTypes[sd->name]  = st;
        structFields[sd->name] = fieldMap;

        // Generate constructor if present
        if (sd->constructor) {
            std::vector<llvm::Type*> paramTypes = { llvm::PointerType::get(context, 0) };
            for (auto& [pname, ptype] : sd->constructor->params)
                paramTypes.push_back(getLLVMType(ptype));

            auto* ctorType = llvm::FunctionType::get(
                llvm::Type::getVoidTy(context), paramTypes, false);
            auto* ctorFunc = llvm::Function::Create(
                ctorType, llvm::Function::ExternalLinkage,
                sd->name + "__ctor", module.get());

            auto* block    = llvm::BasicBlock::Create(context, "entry", ctorFunc);
            auto* oldBlock = builder.GetInsertBlock();
            builder.SetInsertPoint(block);

            auto oldValues = namedValues;
            namedValues.clear();

            // 'self' is the first argument
            auto argIt = ctorFunc->arg_begin();
            llvm::Value* selfPtr = &*argIt++;
            namedValues["self"] = selfPtr;

            int pi = 0;
            for (; argIt != ctorFunc->arg_end(); ++argIt, ++pi) {
                auto& [pname, ptype] = sd->constructor->params[pi];
                auto* alloca = builder.CreateAlloca(argIt->getType(), nullptr, pname);
                builder.CreateStore(&*argIt, alloca);
                namedValues[pname] = alloca;
            }

            for (auto& s : sd->constructor->body)
                generateStmt(s.get());

            if (!builder.GetInsertBlock()->getTerminator())
                builder.CreateRetVoid();

            namedValues = oldValues;
            if (oldBlock) builder.SetInsertPoint(oldBlock);
        }
        return;
    }

    // ── Function Declaration ──────────────────
    if (auto fn = dynamic_cast<FunctionDecl*>(stmt)) {
        std::vector<llvm::Type*> paramTypes;
        for (auto& p : fn->params)
            paramTypes.push_back(getLLVMType(p.second));

        auto funcType = llvm::FunctionType::get(getLLVMType(fn->returnType), paramTypes, false);
        auto function = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, fn->name, module.get()
        );

        auto block     = llvm::BasicBlock::Create(context, "entry", function);
        auto* oldBlock = builder.GetInsertBlock();
        builder.SetInsertPoint(block);

        auto oldValues = namedValues;
        namedValues.clear();

        int idx = 0;
        for (auto& arg : function->args()) {
            auto paramName = fn->params[idx++].first;
            auto alloca    = builder.CreateAlloca(arg.getType(), nullptr, paramName);
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
        if (oldBlock) builder.SetInsertPoint(oldBlock);
        return;
    }

    // ── Return ────────────────────────────────
    if (auto ret = dynamic_cast<ReturnStmt*>(stmt)) {
        auto val = generateExpr(ret->value.get());
        if (val) builder.CreateRet(val);
        else     builder.CreateRetVoid();
        return;
    }

    // ── Variable Declaration ──────────────────
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

    // ── Assignment ────────────────────────────
    if (auto assign = dynamic_cast<AssignmentStmt*>(stmt)) {
        auto value = generateExpr(assign->value.get());
        if (!value) { std::cerr << "[CodeGen] ERROR: assignment RHS is null\n"; return; }

        if (auto var = dynamic_cast<VariableExpr*>(assign->target.get())) {
            auto it = namedValues.find(var->name);
            if (it == namedValues.end()) {
                std::cerr << "[CodeGen] ERROR: assignment to undeclared variable '"
                          << var->name << "'\n";
                return;
            }
            builder.CreateStore(value, it->second);
        }
        return;
    }

    // ── Self Assignment: self.field = value ───
    if (auto sa = dynamic_cast<SelfAssignmentStmt*>(stmt)) {
        auto selfIt = namedValues.find("self");
        if (selfIt == namedValues.end()) {
            std::cerr << "[CodeGen] ERROR: self not in scope\n"; return;
        }
        llvm::Value* selfPtr = selfIt->second;

        // Find which struct we're in via the current function name
        std::string structName;
        if (auto* fn = builder.GetInsertBlock()->getParent()) {
            std::string fname = fn->getName().str();
            auto pos = fname.find("__ctor");
            if (pos != std::string::npos) structName = fname.substr(0, pos);
        }

        auto sit = structTypes.find(structName);
        if (sit == structTypes.end()) {
            std::cerr << "[CodeGen] ERROR: cannot resolve struct for self assignment\n"; return;
        }
        auto* st       = sit->second;
        auto& fieldIdx = structFields[structName];

        int idx = -1;
        for (auto& fi : fieldIdx)
            if (fi.first == sa->field) { idx = fi.second; break; }
        if (idx == -1) {
            std::cerr << "[CodeGen] ERROR: unknown field '" << sa->field << "'\n"; return;
        }

        auto val = generateExpr(sa->value.get());
        if (!val) return;
        auto ptr = builder.CreateStructGEP(st, selfPtr, idx, sa->field + "_ptr");
        builder.CreateStore(val, ptr);
        return;
    }

    // ── Print ─────────────────────────────────
    if (auto print = dynamic_cast<PrintStmt*>(stmt)) {
        auto val = generateExpr(print->expression.get());
        if (!val) { std::cerr << "[CodeGen] ERROR: print expression is null\n"; return; }
        if (val->getType()->isVoidTy()) return;

        auto* type = print->expression->inferredType;

        const char* fmt = nullptr;
        bool needZExt   = false;

        if (type) {
            if      (type->isTensor()) { builder.CreateCall(aiPrintTensorFunc, {val}); return; }
            else if (type->isDouble()) fmt = "%.6g\n";
            else if (type->isString()) fmt = "%s\n";
            else if (type->isBool())   { fmt = "%d\n"; needZExt = true; }
            else                       fmt = "%d\n";
        } else {
            auto* irTy = val->getType();
            if      (irTy->isDoubleTy())   fmt = "%.6g\n";
            else if (irTy->isPointerTy())  fmt = "%s\n";
            else if (irTy->isIntegerTy(1)) { fmt = "%d\n"; needZExt = true; }
            else                           fmt = "%d\n";
        }

        if (needZExt)
            val = builder.CreateZExt(val, llvm::Type::getInt32Ty(context));

        auto fmtVal = builder.CreateGlobalStringPtr(fmt);
        builder.CreateCall(printfFunc, {fmtVal, val});
        return;
    }

    // ── Loop ──────────────────────────────────
    if (auto loop = dynamic_cast<LoopStmt*>(stmt)) {
        auto countVal = generateExpr(loop->count.get());
        if (!countVal) { std::cerr << "[CodeGen] ERROR: loop count is null\n"; return; }

        auto* function = builder.GetInsertBlock()->getParent();

        llvm::Value* prevIterVal = nullptr;
        {
            auto prevIt = namedValues.find(loop->iterator);
            if (prevIt != namedValues.end()) prevIterVal = prevIt->second;
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
        for (auto& s : loop->body) generateStmt(s.get());

        if (!builder.GetInsertBlock()->getTerminator()) {
            auto next = builder.CreateAdd(
                builder.CreateLoad(llvm::Type::getInt32Ty(context), loopVar),
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1)
            );
            builder.CreateStore(next, loopVar);
            builder.CreateBr(condBlock);
        }

        builder.SetInsertPoint(endBlock);

        if (prevIterVal) namedValues[loop->iterator] = prevIterVal;
        else             namedValues.erase(loop->iterator);
        return;
    }

    // ── If Statement ──────────────────────────
    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt)) {
        auto condVal = generateExpr(ifStmt->condition.get());
        if (!condVal) { std::cerr << "[CodeGen] ERROR: if condition is null\n"; return; }

        if (!condVal->getType()->isIntegerTy(1))
            condVal = builder.CreateICmpNE(
                condVal, llvm::ConstantInt::get(condVal->getType(), 0), "bool_cond");

        auto* function  = builder.GetInsertBlock()->getParent();
        auto thenBlock  = llvm::BasicBlock::Create(context, "then",   function);
        auto elseBlock  = llvm::BasicBlock::Create(context, "else",   function);
        auto mergeBlock = llvm::BasicBlock::Create(context, "ifcont", function);

        builder.CreateCondBr(condVal, thenBlock, elseBlock);

        builder.SetInsertPoint(thenBlock);
        for (auto& s : ifStmt->thenBranch) generateStmt(s.get());
        if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(mergeBlock);

        builder.SetInsertPoint(elseBlock);
        for (auto& s : ifStmt->elseBranch) generateStmt(s.get());
        if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(mergeBlock);

        builder.SetInsertPoint(mergeBlock);
        return;
    }

    std::cerr << "[CodeGen] WARNING: unhandled statement type: "
              << typeid(*stmt).name() << "\n";
}

// =============================
// File Expression Generation
// =============================

llvm::Value* CodeGen::generateFileExpr(FileExpr* fe) {
    if (!fe) return nullptr;

    // ── Compile-time read ─────────────────────
    if (fe->compileTime && fe->mode == FileMode::Read) {
        if (auto* strLit = dynamic_cast<StringLiteral*>(fe->path.get())) {
            std::ifstream f(strLit->value);
            if (f.is_open()) {
                std::string contents((std::istreambuf_iterator<char>(f)),
                                      std::istreambuf_iterator<char>());
                return builder.CreateGlobalStringPtr(contents, "ct_file_data");
            }
            std::cerr << "[CodeGen] compile-time warning: cannot open '"
                      << strLit->value << "' — falling back to runtime read\n";
        }
    }

    llvm::Value* pathVal = generateExpr(fe->path.get());
    if (!pathVal) return nullptr;

    switch (fe->mode) {
        case FileMode::Read: {
            auto* fn = module->getFunction("nexa_file_read");
            if (!fn) { std::cerr << "[CodeGen] error: nexa_file_read not declared\n"; return nullptr; }
            return builder.CreateCall(fn, {pathVal}, "file_data");
        }
        case FileMode::Write: {
            auto* fn = module->getFunction("nexa_file_write");
            if (!fn) { std::cerr << "[CodeGen] error: nexa_file_write not declared\n"; return nullptr; }
            auto* content = generateExpr(fe->content.get());
            if (!content) return nullptr;
            builder.CreateCall(fn, {pathVal, content});
            return nullptr;
        }
        case FileMode::Append: {
            auto* fn = module->getFunction("nexa_file_append");
            if (!fn) { std::cerr << "[CodeGen] error: nexa_file_append not declared\n"; return nullptr; }
            auto* content = generateExpr(fe->content.get());
            if (!content) return nullptr;
            builder.CreateCall(fn, {pathVal, content});
            return nullptr;
        }
    }
    return nullptr;
}

// =============================
// Expression Generation
// =============================

llvm::Value* CodeGen::generateExpr(Expr* expr)
{
    if (!expr) return nullptr;

    // ── File expression ───────────────────────
    if (auto fe = dynamic_cast<FileExpr*>(expr))
        return generateFileExpr(fe);

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
        if (!arr->getType()->isPointerTy())
            arr = builder.CreateIntToPtr(arr, llvm::PointerType::get(context, 0), "arr_ptr_cast");
        auto ptr = builder.CreateGEP(llvm::Type::getInt32Ty(context), arr, idx, "elem_ptr");
        return builder.CreateLoad(llvm::Type::getInt32Ty(context), ptr, "elem");
    }

    // ── Array Literal ─────────────────────────
    if (auto arrLit = dynamic_cast<ArrayLiteralExpr*>(expr)) {
        int size   = (int)arrLit->elements.size();
        auto arrTy = llvm::ArrayType::get(llvm::Type::getInt32Ty(context), size);
        auto alloca = builder.CreateAlloca(arrTy, nullptr, "array_tmp");

        for (int i = 0; i < size; ++i) {
            auto val = generateExpr(arrLit->elements[i].get());
            if (!val) continue;
            auto ptr = builder.CreateGEP(arrTy, alloca,
                { builder.getInt32(0), builder.getInt32(i) }, "elem_ptr");
            builder.CreateStore(val, ptr);
        }
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

        auto* lt = bin->left->inferredType;
        auto* rt = bin->right->inferredType;
        if (lt && rt && lt->isTensor() && rt->isTensor() && bin->op == "*")
            return builder.CreateCall(aiMatmulFunc, {L, R}, "matmul_tmp");

        bool lFP = L->getType()->isDoubleTy();
        bool rFP = R->getType()->isDoubleTy();
        if (lFP && !rFP) R = builder.CreateSIToFP(R, llvm::Type::getDoubleTy(context));
        else if (!lFP && rFP) L = builder.CreateSIToFP(L, llvm::Type::getDoubleTy(context));

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

        if (var->inferredType && var->inferredType->isStruct())
            return it->second;

        llvm::Type* loadTy = nullptr;
        if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(it->second))
            loadTy = allocaInst->getAllocatedType();
        else
            loadTy = var->inferredType
                ? getLLVMType(var->inferredType)
                : llvm::Type::getInt32Ty(context);

        return builder.CreateLoad(loadTy, it->second, var->name);
    }

    // ── Function Call ─────────────────────────
    if (auto call = dynamic_cast<CallExpr*>(expr)) {
        std::string funcName = call->callee;

        // Nexa → runtime name mapping
        if      (funcName == "zeros")      funcName = "ai_zeros";
        else if (funcName == "ones")       funcName = "ai_ones";
        else if (funcName == "sum")        funcName = "ai_sum";
        else if (funcName == "mean")       funcName = "ai_mean";
        else if (funcName == "max")        funcName = "ai_max";
        else if (funcName == "min")        funcName = "ai_min";
        else if (funcName == "reshape")    funcName = "ai_reshape";
        else if (funcName == "shape")      funcName = "ai_shape";
        else if (funcName == "get_value")  funcName = "ai_get_value";
        // ── CSV functions ──────────────────────
        else if (funcName == "read_csv")   funcName = "csv_read";
        else if (funcName == "write_csv")  funcName = "csv_write";
        else if (funcName == "csv_rows")   funcName = "csv_rows";
        else if (funcName == "csv_cols")   funcName = "csv_cols";
        else if (funcName == "csv_get")    funcName = "csv_get";
        else if (funcName == "csv_set")    funcName = "csv_set";
        else if (funcName == "csv_row")    funcName = "csv_get_row";
        else if (funcName == "csv_col")    funcName = "csv_get_col";
        else if (funcName == "csv_slice")  funcName = "csv_slice_cols";
        // ── ML functions ───────────────────────
        else if (funcName == "normalize")       funcName = "ml_normalize";
        else if (funcName == "shuffle")         funcName = "ml_shuffle";
        else if (funcName == "train_split")     funcName = "ml_train_split";
        else if (funcName == "test_split")      funcName = "ml_test_split";
        else if (funcName == "hstack")          funcName = "ml_hstack";
        else if (funcName == "LoRe")            funcName = "lore_create";
        else if (funcName == "fit")             funcName = "lore_fit";
        else if (funcName == "predict")         funcName = "lore_predict";
        else if (funcName == "predict_proba")   funcName = "lore_predict_proba";
        else if (funcName == "accuracy")        funcName = "ml_accuracy";
        else if (funcName == "confusion")       funcName = "ml_confusion";
        else if (funcName == "weights")         funcName = "lore_weights";
        else if (funcName == "bias")            funcName = "lore_bias";

        auto* fn = module->getFunction(funcName);
        if (!fn) {
            std::cerr << "[CodeGen] ERROR: unknown function '" << call->callee << "'\n";
            return nullptr;
        }

        std::vector<llvm::Value*> args;
        auto* fnType = fn->getFunctionType();
        for (size_t i = 0; i < call->arguments.size(); ++i) {
            auto val = generateExpr(call->arguments[i].get());
            if (!val) return nullptr;

            // Auto-coerce argument types to match the function signature
            if (i < fnType->getNumParams()) {
                llvm::Type* expected = fnType->getParamType(i);
                llvm::Type* actual   = val->getType();

                if (expected->isFloatTy() && actual->isDoubleTy())
                    val = builder.CreateFPTrunc(val, llvm::Type::getFloatTy(context), "d2f");
                else if (expected->isDoubleTy() && actual->isFloatTy())
                    val = builder.CreateFPExt(val, llvm::Type::getDoubleTy(context), "f2d");
                else if (expected->isIntegerTy(32) && actual->isIntegerTy(1))
                    val = builder.CreateZExt(val, llvm::Type::getInt32Ty(context), "b2i");
            }
            args.push_back(val);
        }
        // Void-returning functions must NOT get a result name — LLVM verifier rejects it
        bool isVoid = fn->getReturnType()->isVoidTy();
        return builder.CreateCall(fn, args, isVoid ? "" : "calltmp");
    }

    // ── Struct Literal ────────────────────────
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
            if (idx == -1) { std::cerr << "[CodeGen] ERROR: unknown field '" << f.first << "'\n"; continue; }
            auto val = generateExpr(f.second.get());
            if (!val) continue;
            auto ptr = builder.CreateStructGEP(st, alloca, idx, f.first + "_ptr");
            builder.CreateStore(val, ptr);
        }
        return alloca;
    }

    // ── Member Access ─────────────────────────
    if (auto ma = dynamic_cast<MemberAccessExpr*>(expr)) {
        auto obj = generateExpr(ma->object.get());
        if (!obj) return nullptr;

        std::string structName;
        if (ma->object->inferredType) structName = ma->object->inferredType->structName;

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

    // ── Constructor Call ──────────────────────
    if (auto cc = dynamic_cast<ConstructorCallExpr*>(expr)) {
        auto* st = structTypes[cc->structName];
        if (!st) {
            std::cerr << "[CodeGen] ERROR: unknown struct '" << cc->structName << "'\n";
            return nullptr;
        }
        auto* alloca   = builder.CreateAlloca(st, nullptr, cc->structName + "_inst");
        auto* ctorFunc = module->getFunction(cc->structName + "__ctor");
        if (ctorFunc) {
            std::vector<llvm::Value*> args = { alloca };
            for (auto& arg : cc->arguments) {
                auto val = generateExpr(arg.get());
                if (!val) return nullptr;
                args.push_back(val);
            }
            builder.CreateCall(ctorFunc, args);
        }
        return alloca;
    }

    std::cerr << "[CodeGen] ERROR: unhandled expression type: "
              << typeid(*expr).name() << "\n";
    return nullptr;
}