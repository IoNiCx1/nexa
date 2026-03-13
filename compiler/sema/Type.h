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
    Tensor
};

struct Type {
    TypeKind kind;
    Type* elementType = nullptr;

    Type(TypeKind k) : kind(k) {}
    Type(TypeKind k, Type* elem) : kind(k), elementType(elem) {}

    bool isInt() const { return kind == TypeKind::Int; }
    bool isDouble() const { return kind == TypeKind::Double; }
    bool isString() const { return kind == TypeKind::String; }
    bool isBool() const { return kind == TypeKind::Bool; }
    bool isArray() const { return kind == TypeKind::Array; }
    bool isVoid() const { return kind == TypeKind::Void; }
    bool isTensor() const { return kind == TypeKind::Tensor; }

    std::string toString() const {
        switch (kind) {
            case TypeKind::Int: return "int";
            case TypeKind::Double: return "double";
            case TypeKind::String: return "string";
            case TypeKind::Bool: return "bool";
            case TypeKind::Array: return elementType->toString() + "[]";
            case TypeKind::Void: return "void";
            case TypeKind::Tensor: return "tensor";
        }
        return "unknown";
    }
};

static Type TYPE_INT(TypeKind::Int);
static Type TYPE_DOUBLE(TypeKind::Double);
static Type TYPE_STRING(TypeKind::String);
static Type TYPE_BOOL(TypeKind::Bool);
static Type TYPE_VOID(TypeKind::Void);
static Type TYPE_TENSOR(TypeKind::Tensor); // ✅ Added missing instance

} // namespace nexa

#endif