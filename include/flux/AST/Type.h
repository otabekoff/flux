#pragma once

#include "flux/AST/AST.h"

#include <optional>
#include <string>
#include <vector>

namespace flux {
namespace ast {

// ============================================================================
// Type AST Nodes
// ============================================================================

/// Base class for type representations in the AST.
class TypeNode : public ASTNode {
public:
    enum class Kind {
        Named,      // Int32, String, User, etc.
        Generic,    // Array<Int32>, HashMap<String, Int32>
        Reference,  // ref T
        MutRef,     // mut ref T
        Tuple,      // (Int32, String)
        Function,   // (Int32, Int32) -> Int32
        Array,      // Array<T, N> (fixed size)
        Option,     // Option<T>
        Result,     // Result<T, E>
        Inferred,   // _ (placeholder, not used in Flux but for internal)
    };

    Kind kind;
    virtual ~TypeNode() = default;

protected:
    explicit TypeNode(Kind k, SourceLocation loc = {})
        : ASTNode(loc), kind(k) {}
};

/// A named type like Int32, String, User, MyStruct
class NamedType : public TypeNode {
public:
    /// Path segments, e.g. ["std", "collections", "HashMap"]
    std::vector<std::string> path;

    /// The simple name (last segment) for convenience
    const std::string& name() const { return path.back(); }

    explicit NamedType(std::vector<std::string> path, SourceLocation loc = {})
        : TypeNode(Kind::Named, loc), path(std::move(path)) {}

    static bool classof(const TypeNode* t) {
        return t->kind == Kind::Named;
    }
};

/// A generic type like Array<Int32>, HashMap<String, Int32>
class GenericType : public TypeNode {
public:
    std::unique_ptr<NamedType> base;              // The base type name
    std::vector<TypeNodePtr> typeArguments;        // Type parameters

    GenericType(std::unique_ptr<NamedType> base,
                std::vector<TypeNodePtr> args,
                SourceLocation loc = {})
        : TypeNode(Kind::Generic, loc),
          base(std::move(base)),
          typeArguments(std::move(args)) {}

    static bool classof(const TypeNode* t) {
        return t->kind == Kind::Generic;
    }
};

/// A reference type: ref T
class ReferenceType : public TypeNode {
public:
    TypeNodePtr inner;
    std::optional<std::string> lifetime; // e.g., 'a

    explicit ReferenceType(TypeNodePtr inner, SourceLocation loc = {})
        : TypeNode(Kind::Reference, loc), inner(std::move(inner)) {}

    static bool classof(const TypeNode* t) {
        return t->kind == Kind::Reference;
    }
};

/// A mutable reference type: mut ref T
class MutRefType : public TypeNode {
public:
    TypeNodePtr inner;
    std::optional<std::string> lifetime;

    explicit MutRefType(TypeNodePtr inner, SourceLocation loc = {})
        : TypeNode(Kind::MutRef, loc), inner(std::move(inner)) {}

    static bool classof(const TypeNode* t) {
        return t->kind == Kind::MutRef;
    }
};

/// A tuple type: (Int32, String, Float64)
class TupleType : public TypeNode {
public:
    std::vector<TypeNodePtr> elements;

    explicit TupleType(std::vector<TypeNodePtr> elements, SourceLocation loc = {})
        : TypeNode(Kind::Tuple, loc), elements(std::move(elements)) {}

    static bool classof(const TypeNode* t) {
        return t->kind == Kind::Tuple;
    }
};

/// A function type: (Int32, Int32) -> Int32
class FunctionType : public TypeNode {
public:
    std::vector<TypeNodePtr> paramTypes;
    TypeNodePtr returnType;

    FunctionType(std::vector<TypeNodePtr> params, TypeNodePtr ret,
                 SourceLocation loc = {})
        : TypeNode(Kind::Function, loc),
          paramTypes(std::move(params)),
          returnType(std::move(ret)) {}

    static bool classof(const TypeNode* t) {
        return t->kind == Kind::Function;
    }
};

/// Array type with fixed size: Array<Int32, 5>
class ArrayType : public TypeNode {
public:
    TypeNodePtr elementType;
    std::optional<uint64_t> size; // None for dynamic arrays (Vector)

    ArrayType(TypeNodePtr elemType, std::optional<uint64_t> size,
              SourceLocation loc = {})
        : TypeNode(Kind::Array, loc),
          elementType(std::move(elemType)),
          size(size) {}

    static bool classof(const TypeNode* t) {
        return t->kind == Kind::Array;
    }
};

} // namespace ast
} // namespace flux
