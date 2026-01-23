#include "SymbolTable.h"
#include "../ast/Ast.h"
#include <stdexcept>

class SemanticAnalyzer {
public:
    void analyze(Program& program) {
        for (auto& stmt : program.statements) {
            analyzeDeclaration(static_cast<Declaration&>(*stmt));
        }
    }

private:
    SymbolTable symbols;

    SemanticType lowerType(const TypeSpec& t) {
        return SemanticType{ t.kind, t.rows, t.cols };
    }

    void analyzeDeclaration(Declaration& decl) {
        if (decl.names.size() != decl.values.size()) {
            throw std::runtime_error("Variable count does not match value count");
        }

        SemanticType declType = lowerType(decl.type);

        for (size_t i = 0; i < decl.names.size(); ++i) {
            const std::string& name = decl.names[i];

            if (symbols.exists(name)) {
                throw std::runtime_error("Redeclaration of variable: " + name);
            }

            symbols.insert(name, declType);
        }
    }
};
