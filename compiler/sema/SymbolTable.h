#pragma once
#include "Type.h"
#include <unordered_map>
#include <string>

struct Symbol {
    SemanticType type;
    bool mutableFlag;
};

class SymbolTable {
public:
    bool exists(const std::string& name) const {
        return table.count(name);
    }

    void insert(const std::string& name, const SemanticType& type) {
        table[name] = { type, true };
    }

    const Symbol& lookup(const std::string& name) const {
        return table.at(name);
    }

private:
    std::unordered_map<std::string, Symbol> table;
};
