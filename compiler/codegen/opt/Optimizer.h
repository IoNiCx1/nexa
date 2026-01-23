#pragma once
#include "../LLVMContext.h"

/*
  LLVM optimization pipeline for NEXA
*/
class Optimizer {
public:
    explicit Optimizer(LLVMState& state);

    void run();

private:
    LLVMState& llvm;
};
