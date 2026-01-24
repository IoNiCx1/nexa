#include "SemanticAnalyzer.h"
#include <stdexcept>

void SemanticAnalyzer::analyze(Program& program) {
    for (auto& stmt : program.statements) {
        // Attempt to treat the statement as a variable declaration
        if (auto* varDecl = dynamic_cast<VarDecl*>(stmt.get())) {
            analyzeVarDecl(*varDecl);
        }
    }
}

SemanticType SemanticAnalyzer::lowerType(const TypeSpec& t) {
    // Maps the AST TypeSpec fields (kind, rows, cols) to the SemanticType
    return SemanticType{ t.kind, t.rows, t.cols };
}

void SemanticAnalyzer::analyzeVarDecl(VarDecl& decl) {
    const std::string& name = decl.name;

    // Check for redeclaration
    if (symbols.exists(name)) {
        throw std::runtime_error("Semantic Error: Redeclaration of variable '" + name + "'");
    }

    // Register in symbol table
    SemanticType declType = lowerType(decl.type);
    symbols.insert(name, declType);
}
