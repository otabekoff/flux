#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Decl.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Pattern.h"
#include "flux/AST/Stmt.h"
#include "flux/AST/Type.h"
#include "flux/Common/Diagnostics.h"
#include "flux/Lexer/Lexer.h"
#include "flux/Lexer/Token.h"

#include <memory>
#include <string>
#include <vector>

namespace flux {

/// Recursive-descent parser for the Flux programming language.
///
/// Produces an AST from a stream of tokens. All types must be explicitly
/// annotated per the Flux specification (no type inference).
///
/// Grammar precedence (lowest to highest):
///   Assignment   =, +=, -=, etc.
///   Or           or
///   And          and
///   Equality     ==, !=
///   Comparison   <, <=, >, >=
///   BitOr        |
///   BitXor       ^
///   BitAnd       &
///   Shift        <<, >>
///   Additive     +, -
///   Multiplicative *, /, %
///   Unary        -, not, ~, ref, mut ref, move, await
///   Postfix      (), [], ., ::, ?, as
///   Primary      literals, identifiers, paths, groups
///
class Parser {
public:
  Parser(Lexer &lexer, DiagnosticEngine &diag);

  /// Parse a complete module (entry point).
  std::unique_ptr<ast::Module> parseModule();

private:
  // ---- Token management ----
  Token current() const { return current_; }
  Token advance();
  bool check(TokenKind kind) const;
  bool match(TokenKind kind);
  Token expect(TokenKind kind, const std::string &message);
  Token expectSemicolon();

  // ---- Top-level declarations ----
  ast::DeclPtr parseDeclaration();
  std::unique_ptr<ast::ModuleDecl> parseModuleDecl();
  std::unique_ptr<ast::ImportDecl> parseImportDecl();
  std::unique_ptr<ast::FuncDecl> parseFuncDecl(bool isAsync = false);
  std::unique_ptr<ast::StructDecl> parseStructDecl();
  std::unique_ptr<ast::ClassDecl> parseClassDecl();
  std::unique_ptr<ast::EnumDecl> parseEnumDecl();
  std::unique_ptr<ast::TraitDecl> parseTraitDecl();
  std::unique_ptr<ast::ImplDecl> parseImplDecl();
  std::unique_ptr<ast::TypeAliasDecl> parseTypeAliasDecl();

  // ---- Helpers for declarations ----
  std::vector<ast::GenericParam> parseGenericParams();
  std::vector<ast::FuncParam> parseFuncParams();
  ast::FuncParam parseFuncParam();
  std::vector<ast::FieldDecl> parseStructFields();
  std::vector<ast::FieldDecl> parseClassFields();
  std::vector<ast::EnumVariant> parseEnumVariants();
  std::vector<std::string> parsePath();

  // ---- Statements ----
  ast::StmtPtr parseStatement();
  std::unique_ptr<ast::LetStmt> parseLetStmt();
  std::unique_ptr<ast::ConstStmt> parseConstStmt();
  std::unique_ptr<ast::ReturnStmt> parseReturnStmt();
  std::unique_ptr<ast::IfStmt> parseIfStmt();
  std::unique_ptr<ast::MatchStmt> parseMatchStmt();
  std::unique_ptr<ast::ForStmt> parseForStmt();
  std::unique_ptr<ast::WhileStmt> parseWhileStmt();
  std::unique_ptr<ast::LoopStmt> parseLoopStmt();
  std::unique_ptr<ast::BlockStmt> parseBlock();

  // ---- Expressions (Pratt parser / precedence climbing) ----
  ast::ExprPtr parseExpression();
  ast::ExprPtr parseAssignment();
  ast::ExprPtr parseOr();
  ast::ExprPtr parseAnd();
  ast::ExprPtr parseEquality();
  ast::ExprPtr parseComparison();
  ast::ExprPtr parseBitwiseOr();
  ast::ExprPtr parseBitwiseXor();
  ast::ExprPtr parseBitwiseAnd();
  ast::ExprPtr parseShift();
  ast::ExprPtr parseAdditive();
  ast::ExprPtr parseMultiplicative();
  ast::ExprPtr parseUnary();
  ast::ExprPtr parsePostfix(ast::ExprPtr left);
  ast::ExprPtr parsePrimary();

  // ---- Expression helpers ----
  ast::ExprPtr parseCallArguments(ast::ExprPtr callee);
  ast::ExprPtr parseIndexExpression(ast::ExprPtr object);
  ast::ExprPtr parseIfExpr();
  ast::ExprPtr parseMatchExpr();
  ast::ExprPtr parseBlockExpr();
  ast::ExprPtr parseClosureExpr();
  ast::ExprPtr parseConstructExpr(ast::ExprPtr typePath);

  // ---- Match arms ----
  ast::MatchArm parseMatchArm();

  // ---- Patterns ----
  ast::PatternPtr parsePattern();

  // ---- Types ----
  ast::TypeNodePtr parseType();
  ast::TypeNodePtr parseNamedOrGenericType();
  ast::TypeNodePtr parseTupleType();
  ast::TypeNodePtr parseRefType();

  // ---- Error recovery ----
  void synchronize();

  // ---- Lookahead / state snapshot ----
  struct ParserState {
    Token current;
    Token previous;
    uint32_t lexerPos;
    uint32_t lexerLine;
    uint32_t lexerColumn;
  };
  ParserState saveState() const;
  void restoreState(const ParserState &state);

  // ---- State ----
  Lexer &lexer_;
  DiagnosticEngine &diag_;
  Token current_;
  Token previous_;
};

} // namespace flux
