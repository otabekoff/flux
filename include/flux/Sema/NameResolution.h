#pragma once

#include "flux/AST/Decl.h"
#include "flux/AST/Type.h"
#include "flux/Common/Diagnostics.h"
#include "flux/Common/SourceLocation.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace flux {

/// A symbol in the symbol table.
struct Symbol {
    enum class Kind {
        Variable,
        Function,
        Struct,
        Class,
        Enum,
        Trait,
        TypeAlias,
        GenericParam,
        Module,
        EnumVariant,
    };

    Kind kind;
    std::string name;
    std::string qualifiedName;     // fully qualified path
    SourceLocation location;
    ast::Decl::Visibility visibility = ast::Decl::Visibility::Public;

    // For variables
    bool isMutable = false;
    bool isConst = false;
    std::string typeName;          // the declared type as string

    // For functions
    std::vector<std::string> paramTypes;
    std::string returnType;
    bool isAsync = false;

    // For generic types
    std::vector<std::string> genericParams;
};

/// A scope in the symbol table hierarchy.
struct Scope {
    std::string name;
    std::unordered_map<std::string, Symbol> symbols;
    Scope* parent = nullptr;
    std::vector<std::unique_ptr<Scope>> children;

    Scope() = default;
    explicit Scope(const std::string& name, Scope* parent = nullptr)
        : name(name), parent(parent) {}

    /// Look up a symbol in this scope and parent scopes.
    Symbol* lookup(const std::string& name);
    const Symbol* lookup(const std::string& name) const;

    /// Insert a symbol into this scope.
    bool insert(const std::string& name, Symbol symbol);
};

/// Name resolution pass.
/// Resolves all identifiers to their declarations, builds the symbol table.
class NameResolver {
public:
    NameResolver(DiagnosticEngine& diag, Scope& rootScope);

    /// Resolve names in a complete module.
    void resolve(ast::Module& module);

private:
    void registerDecl(ast::Decl& decl);
    void resolveDecl(ast::Decl& decl);
    void resolveStmt(ast::Stmt& stmt);
    void resolveExpr(ast::Expr& expr);

    void resolveImport(ast::ImportDecl& decl);
    void resolveFunc(ast::FuncDecl& decl);
    void resolveStruct(ast::StructDecl& decl);
    void resolveClass(ast::ClassDecl& decl);
    void resolveEnum(ast::EnumDecl& decl);
    void resolveTrait(ast::TraitDecl& decl);
    void resolveImpl(ast::ImplDecl& decl);
    void resolveTypeAlias(ast::TypeAliasDecl& decl);

    void enterScope(const std::string& name);
    void exitScope();

    DiagnosticEngine& diag_;
    Scope* currentScope_ = nullptr;
};

} // namespace flux
