#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Stmt.h"
#include "flux/AST/Type.h"

#include <optional>
#include <string>
#include <vector>

namespace flux {
namespace ast {

// ============================================================================
// Declaration AST Nodes
// ============================================================================

/// Base class for all declarations.
class Decl : public ASTNode {
public:
    enum class Kind {
        Module,
        Import,
        Func,
        Struct,
        Class,
        Enum,
        Trait,
        Impl,
        TypeAlias,
    };

    Kind kind;
    virtual ~Decl() = default;

    /// Visibility
    enum class Visibility { Private, Public };
    Visibility visibility = Visibility::Private;

protected:
    explicit Decl(Kind k, SourceLocation loc = {})
        : ASTNode(loc), kind(k) {}
};

// --- Module declaration ---
// module my_project::services::user_service;
class ModuleDecl : public Decl {
public:
    std::vector<std::string> path; // ["my_project", "services", "user_service"]

    explicit ModuleDecl(std::vector<std::string> path, SourceLocation loc = {})
        : Decl(Kind::Module, loc), path(std::move(path)) {}
};

// --- Import declaration ---
// import std::collections::HashMap;
class ImportDecl : public Decl {
public:
    std::vector<std::string> path;
    std::optional<std::string> alias; // import X as Y

    ImportDecl(std::vector<std::string> path,
               std::optional<std::string> alias = std::nullopt,
               SourceLocation loc = {})
        : Decl(Kind::Import, loc), path(std::move(path)),
          alias(std::move(alias)) {}
};

// --- Generic type parameter ---
struct GenericParam {
    std::string name;                        // T, E, etc.
    std::vector<std::string> traitBounds;    // Comparable, Clone, etc.
    std::optional<std::string> lifetime;     // 'a
    SourceLocation location;
};

// --- Function parameter ---
struct FuncParam {
    std::string name;
    TypeNodePtr type;
    bool isMutable = false;    // mut self, mut ref
    bool isSelf = false;       // self: T
    bool isRef = false;        // ref T
    bool isMutRef = false;     // mut ref T
    SourceLocation location;
};

// --- Function declaration ---
// func name<T: Bound>(params) -> ReturnType { body }
class FuncDecl : public Decl {
public:
    std::string name;
    std::vector<GenericParam> genericParams;
    std::vector<FuncParam> params;
    TypeNodePtr returnType;      // nullptr for Void
    std::unique_ptr<BlockStmt> body;  // nullptr for trait method declarations
    bool isAsync = false;
    bool isUnsafe = false;

    FuncDecl(std::string name, std::vector<FuncParam> params,
             TypeNodePtr returnType, std::unique_ptr<BlockStmt> body,
             SourceLocation loc = {})
        : Decl(Kind::Func, loc), name(std::move(name)),
          params(std::move(params)), returnType(std::move(returnType)),
          body(std::move(body)) {}
};

// --- Struct field ---
struct FieldDecl {
    std::string name;
    TypeNodePtr type;
    Decl::Visibility visibility = Decl::Visibility::Public;
    SourceLocation location;
};

// --- Struct declaration ---
// struct Point { x: Float64, y: Float64 }
class StructDecl : public Decl {
public:
    std::string name;
    std::vector<GenericParam> genericParams;
    std::vector<FieldDecl> fields;

    StructDecl(std::string name, std::vector<FieldDecl> fields,
               SourceLocation loc = {})
        : Decl(Kind::Struct, loc), name(std::move(name)),
          fields(std::move(fields)) {}
};

// --- Class declaration ---
// class User { private id: Int32, public name: String }
class ClassDecl : public Decl {
public:
    std::string name;
    std::vector<GenericParam> genericParams;
    std::vector<FieldDecl> fields;
    std::vector<std::unique_ptr<FuncDecl>> methods;

    ClassDecl(std::string name, std::vector<FieldDecl> fields,
              SourceLocation loc = {})
        : Decl(Kind::Class, loc), name(std::move(name)),
          fields(std::move(fields)) {}
};

// --- Enum variant ---
struct EnumVariant {
    std::string name;

    enum class VariantKind { Unit, Tuple, Struct };
    VariantKind variantKind = VariantKind::Unit;

    // For tuple variants: Move { x: Int32, y: Int32 } -> unnamed fields
    std::vector<TypeNodePtr> tupleFields;

    // For struct variants: Move { x: Int32, y: Int32 } -> named fields
    std::vector<FieldDecl> structFields;

    SourceLocation location;
};

// --- Enum declaration ---
// enum Direction { North, South, East, West }
// enum Message { Quit, Move { x: Int32, y: Int32 }, Write(String) }
class EnumDecl : public Decl {
public:
    std::string name;
    std::vector<GenericParam> genericParams;
    std::vector<EnumVariant> variants;

    EnumDecl(std::string name, std::vector<EnumVariant> variants,
             SourceLocation loc = {})
        : Decl(Kind::Enum, loc), name(std::move(name)),
          variants(std::move(variants)) {}
};

// --- Trait declaration ---
// trait Drawable { func draw(self: Self) -> Void; func area(self: Self) -> Float64; }
class TraitDecl : public Decl {
public:
    std::string name;
    std::vector<GenericParam> genericParams;
    std::vector<std::string> superTraits; // trait bounds this trait extends
    std::vector<std::unique_ptr<FuncDecl>> methods;

    TraitDecl(std::string name,
              std::vector<std::unique_ptr<FuncDecl>> methods,
              SourceLocation loc = {})
        : Decl(Kind::Trait, loc), name(std::move(name)),
          methods(std::move(methods)) {}
};

// --- Impl declaration ---
// impl Point { ... }
// impl Drawable for Circle { ... }
class ImplDecl : public Decl {
public:
    TypeNodePtr targetType;                         // The type being implemented
    std::optional<std::string> traitName;           // For trait implementations
    std::vector<GenericParam> genericParams;
    std::vector<std::unique_ptr<FuncDecl>> methods;

    ImplDecl(TypeNodePtr targetType,
             std::optional<std::string> traitName,
             std::vector<std::unique_ptr<FuncDecl>> methods,
             SourceLocation loc = {})
        : Decl(Kind::Impl, loc), targetType(std::move(targetType)),
          traitName(std::move(traitName)),
          methods(std::move(methods)) {}
};

// --- Type alias ---
// type UserId = Int32;
class TypeAliasDecl : public Decl {
public:
    std::string name;
    std::vector<GenericParam> genericParams;
    TypeNodePtr aliasedType;

    TypeAliasDecl(std::string name, TypeNodePtr aliasedType,
                  SourceLocation loc = {})
        : Decl(Kind::TypeAlias, loc), name(std::move(name)),
          aliasedType(std::move(aliasedType)) {}
};

} // namespace ast
} // namespace flux
