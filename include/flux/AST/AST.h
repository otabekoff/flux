#pragma once

#include "flux/Common/SourceLocation.h"
#include "flux/Lexer/Token.h"

#include <memory>
#include <string>
#include <vector>

/// Forward declarations and base classes for all Flux AST nodes.

namespace flux {
namespace ast {

// Forward declarations
class Decl;
class Stmt;
class Expr;
class TypeNode;
class Pattern;

// Convenience aliases
using DeclPtr = std::unique_ptr<Decl>;
using StmtPtr = std::unique_ptr<Stmt>;
using ExprPtr = std::unique_ptr<Expr>;
using TypeNodePtr = std::unique_ptr<TypeNode>;
using PatternPtr = std::unique_ptr<Pattern>;

using DeclList = std::vector<DeclPtr>;
using StmtList = std::vector<StmtPtr>;
using ExprList = std::vector<ExprPtr>;

// ============================================================================
// AST Node base class
// ============================================================================

/// Base class for all AST nodes.
class ASTNode {
public:
  virtual ~ASTNode() = default;

  SourceLocation location;

protected:
  ASTNode() = default;
  explicit ASTNode(SourceLocation loc) : location(loc) {}
};

// ============================================================================
// Module (top-level translation unit)
// ============================================================================

/// Represents a complete Flux source file / module.
struct Module {
  std::string name; // e.g., "my_project::services::user_service"
  std::vector<std::string> imports; // fully qualified import paths
  DeclList declarations;            // top-level declarations
  SourceLocation location;
};

} // namespace ast
} // namespace flux
