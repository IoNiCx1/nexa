#ifndef NEXA_TYPE_H
#define NEXA_TYPE_H

#include <string>

namespace nexa {

// =============================
// Type Kinds
// =============================

enum class TypeKind {
    Int,
    Double,
    String,
    Array,
    Void
};

// =============================
// Base Type
// =============================

struct Type {
    TypeKind kind;

    // For Array types
    Type* elementType = nullptr;

    Type(TypeKind k) : kind(k) {}
    Type(TypeKind k, Type* elem)
        : kind(k), elementType(elem) {}

    bool isInt() const { return kind == TypeKind::Int; }
    bool isDouble() const { return kind == TypeKind::Double; }
    bool isString() const { return kind == TypeKind::String; }
    bool isArray() const { return kind == TypeKind::Array; }
    bool isVoid() const { return kind == TypeKind::Void; }

    std::string toString() const {
        switch (kind) {
            case TypeKind::Int: return "int";
            case TypeKind::Double: return "double";
            case TypeKind::String: return "string";
            case TypeKind::Array:
                return elementType->toString() + "[]";
            case TypeKind::Void:
                return "void";
        }
        return "unknown";
    }
};

// =============================
// Global Primitive Instances
// =============================

static Type TYPE_INT(TypeKind::Int);
static Type TYPE_DOUBLE(TypeKind::Double);
static Type TYPE_STRING(TypeKind::String);
static Type TYPE_VOID(TypeKind::Void);

} // namespace nexa

#endif