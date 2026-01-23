#include "Optimizer.h"

#include <llvm/Passes/PassBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Vectorize.h>

Optimizer::Optimizer(LLVMState& state)
    : llvm(state) {}

void Optimizer::run() {
    using namespace llvm;

    PassBuilder passBuilder;

    LoopAnalysisManager loopAM;
    FunctionAnalysisManager funcAM;
    CGSCCAnalysisManager cgsccAM;
    ModuleAnalysisManager modAM;

    passBuilder.registerModuleAnalyses(modAM);
    passBuilder.registerCGSCCAnalyses(cgsccAM);
    passBuilder.registerFunctionAnalyses(funcAM);
    passBuilder.registerLoopAnalyses(loopAM);
    passBuilder.crossRegisterProxies(
        loopAM, funcAM, cgsccAM, modAM);

    ModulePassManager modulePM;

    // ---- Core optimizations ----
    modulePM.addPass(createModuleToFunctionPassAdaptor(
        SimplifyCFGPass()));

    modulePM.addPass(createModuleToFunctionPassAdaptor(
        InstCombinePass()));

    modulePM.addPass(createModuleToFunctionPassAdaptor(
        ReassociatePass()));

    modulePM.addPass(createModuleToFunctionPassAdaptor(
        EarlyCSEPass()));

    // ---- Loop optimizations (CRITICAL) ----
    modulePM.addPass(createModuleToFunctionPassAdaptor(
        LoopSimplifyPass()));

    modulePM.addPass(createModuleToFunctionPassAdaptor(
        LoopRotatePass()));

    modulePM.addPass(createModuleToFunctionPassAdaptor(
        LoopUnrollPass()));

    modulePM.addPass(createModuleToFunctionPassAdaptor(
        LoopVectorizePass()));

    // ---- SLP vectorization ----
    modulePM.addPass(createModuleToFunctionPassAdaptor(
        SLPVectorizerPass()));

    // ---- Cleanup ----
    modulePM.addPass(createModuleToFunctionPassAdaptor(
        DCEPass()));

    modulePM.run(*llvm.module, modAM);
}
