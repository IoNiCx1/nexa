#ifndef NEXA_TYPE_H
#define NEXA_TYPE_H

#include <string>

namespace nexa {

enum class TypeKind { Int, Double, String, Void, Invalid };

class Type {
public:
  TypeKind kind;

  Type(TypeKind k = TypeKind::Invalid) : kind(k) {}

  static Type getInt() { return Type(TypeKind::Int); }
  static Type getDouble() { return Type(TypeKind::Double); }
  static Type getString() { return Type(TypeKind::String); }
  static Type getVoid() { return Type(TypeKind::Void); }
  static Type getInvalid() { return Type(TypeKind::Invalid); }

  bool isNumeric() const {
    return kind == TypeKind::Int || kind == TypeKind::Double;
  }

  bool isInt() const { return kind == TypeKind::Int; }
  bool isDouble() const { return kind == TypeKind::Double; }
  bool isString() const { return kind == TypeKind::String; }
  bool isVoid() const { return kind == TypeKind::Void; }

  bool operator==(const Type &other) const { return kind == other.kind; }

  bool operator!=(const Type &other) const { return kind != other.kind; }

  std::string toString() const {
    switch (kind) {
    case TypeKind::Int:
      return "int";
    case TypeKind::Double:
      return "double";
    case TypeKind::String:
      return "string";
    case TypeKind::Void:
      return "void";
    default:
      return "invalid";
    }
  }
};

} // namespace nexa

#endif