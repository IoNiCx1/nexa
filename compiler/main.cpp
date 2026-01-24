#include <iostream>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sema/SemanticAnalyzer.h"
#include "ir/CodeGen.h"
#include "ir/LLVMContext.h"

int main() {
    std::string source = "int x = 42;";
    Lexer lexer(source);
    Parser parser(lexer);
    
    auto program = parser.parseProgram();
    
    if (program) {
        // 1. Semantic Analysis
        SemanticAnalyzer sema;
        sema.analyze(*program); // Dereference unique_ptr to get Program&

        // 2. Code Generation
        LLVMState state; 
        CodeGen generator(state);
        generator.generate(*program); // Dereference unique_ptr to get Program&
        
        std::cout << "Successfully compiled!" << std::endl;
    }

    return 0;
}
