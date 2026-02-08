#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Decl.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Stmt.h"
#include "flux/AST/Type.h"
#include "flux/Common/Diagnostics.h"
#include "flux/Sema/NameResolution.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace flux {

/// Type checker for the Flux language.
///
/// Since Flux requires all types to be explicitly declared (no inference),
/// the type checker validates that:
/// - All declared types exist and are valid
/// - Expressions match their expected types
/// - Function arguments match parameter types
/// - Return types are consistent
/// - Generic type arguments satisfy trait bounds
/// - Assignment targets are mutable
///
class TypeChecker {
public:
    TypeChecker(DiagnosticEngine& diag, const Scope& scope);

    /// Check types for an entire module.
    void check(ast::Module& module);

private:
    // Declaration checking
    void checkDecl(ast::Decl& decl);
    void checkFuncDecl(ast::FuncDecl& decl);
    void checkStructDecl(ast::StructDecl& decl);
    void checkClassDecl(ast::ClassDecl& decl);
    void checkEnumDecl(ast::EnumDecl& decl);
    void checkTraitDecl(ast::TraitDecl& decl);
    void checkImplDecl(ast::ImplDecl& decl);

    // Statement checking
    void checkStmt(ast::Stmt& stmt);
    void checkLetStmt(ast::LetStmt& stmt);
    void checkReturnStmt(ast::ReturnStmt& stmt);
    void checkIfStmt(ast::IfStmt& stmt);
    void checkForStmt(ast::ForStmt& stmt);
    void checkWhileStmt(ast::WhileStmt& stmt);
    void checkBlockStmt(ast::BlockStmt& stmt);

    // Expression checking — returns the inferred type name
    std::string checkExpr(ast::Expr& expr);
    std::string checkBinaryExpr(ast::BinaryExpr& expr);
    std::string checkCallExpr(ast::CallExpr& expr);
    std::string checkIdentExpr(ast::IdentExpr& expr);

    // Type utilities
    bool isValidType(const std::string& typeName) const;
    bool typesCompatible(const std::string& expected,
                         const std::string& actual) const;
    std::string typeToString(const ast::TypeNode& type) const;

    // Built-in type registry
    void registerBuiltinTypes();

    DiagnosticEngine& diag_;
    const Scope& scope_;

    // Known types (built-in + user-defined)
    std::unordered_set<std::string> knownTypes_;

    // Current function return type (for checking return statements)
    std::string currentReturnType_;
};

} // namespace flux
