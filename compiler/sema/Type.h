#ifndef NEXA_TYPE_H
#define NEXA_TYPE_H

#include <string>

namespace nexa {

enum class TypeKind {
    Int,
    Double,
    String,
    Bool,
    Array,
    Void,
    Tensor,
    Struct
};

struct Type {
    TypeKind kind;
    Type*    elementType = nullptr;
    std::string structName;

    Type(TypeKind k)                        : kind(k), elementType(nullptr) {}
    Type(TypeKind k, Type* elem)            : kind(k), elementType(elem)    {}
    Type(TypeKind k, const std::string& sn) : kind(k), structName(sn) {}

    bool isInt()    const { return kind == TypeKind::Int;    }
    bool isDouble() const { return kind == TypeKind::Double; }
    bool isString() const { return kind == TypeKind::String; }
    bool isBool()   const { return kind == TypeKind::Bool;   }
    bool isArray()  const { return kind == TypeKind::Array;  }
    bool isVoid()   const { return kind == TypeKind::Void;   }
    bool isTensor() const { return kind == TypeKind::Tensor; }
    bool isStruct() const { return kind == TypeKind::Struct; }

    std::string toString() const {
        switch (kind) {
            case TypeKind::Int:    return "int";
            case TypeKind::Double: return "double";
            case TypeKind::String: return "string";
            case TypeKind::Bool:   return "bool";
            case TypeKind::Array:  return (elementType ? elementType->toString() : "?") + "[]";
            case TypeKind::Void:   return "void";
            case TypeKind::Tensor: return "tensor";
            case TypeKind::Struct: return structName;
        }
        return "unknown";
    }
};

// ── Singleton type instances ───────────────────────────────────────────────
// Declared extern here so every translation unit shares the SAME object.
// Defined exactly once in Type.cpp.
// Using 'static' in a header gives each TU its own copy → ODR violation.
extern Type TYPE_INT;
extern Type TYPE_DOUBLE;
extern Type TYPE_STRING;
extern Type TYPE_BOOL;
extern Type TYPE_VOID;
extern Type TYPE_TENSOR;
extern Type TYPE_INT_ARRAY;   // int[]

} // namespace nexa

#endif