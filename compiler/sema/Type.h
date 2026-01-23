#pragma once
#include "../ast/Ast.h"

/*
  Semantic type wrapper
  Used after parsing, before LLVM
*/
struct SemanticType {
    TypeKind kind;
    int rows;
    int cols;

    bool operator==(const SemanticType& other) const {
        return kind == other.kind &&
               rows == other.rows &&
               cols == other.cols;
    }
};
