#pragma once

#include "flux/AST/Decl.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Pattern.h"
#include "flux/AST/Stmt.h"
#include "flux/AST/Type.h"

namespace flux {
namespace ast {

/// Visitor pattern for traversing all AST node types.
/// Override the visit methods you're interested in.
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // ---- Declarations ----
    virtual void visit(ModuleDecl& node) {}
    virtual void visit(ImportDecl& node) {}
    virtual void visit(FuncDecl& node) {}
    virtual void visit(StructDecl& node) {}
    virtual void visit(ClassDecl& node) {}
    virtual void visit(EnumDecl& node) {}
    virtual void visit(TraitDecl& node) {}
    virtual void visit(ImplDecl& node) {}
    virtual void visit(TypeAliasDecl& node) {}

    // ---- Statements ----
    virtual void visit(LetStmt& node) {}
    virtual void visit(ConstStmt& node) {}
    virtual void visit(ReturnStmt& node) {}
    virtual void visit(IfStmt& node) {}
    virtual void visit(MatchStmt& node) {}
    virtual void visit(ForStmt& node) {}
    virtual void visit(WhileStmt& node) {}
    virtual void visit(LoopStmt& node) {}
    virtual void visit(BreakStmt& node) {}
    virtual void visit(ContinueStmt& node) {}
    virtual void visit(BlockStmt& node) {}
    virtual void visit(ExprStmt& node) {}

    // ---- Expressions ----
    virtual void visit(IntLiteralExpr& node) {}
    virtual void visit(FloatLiteralExpr& node) {}
    virtual void visit(StringLiteralExpr& node) {}
    virtual void visit(CharLiteralExpr& node) {}
    virtual void visit(BoolLiteralExpr& node) {}
    virtual void visit(IdentExpr& node) {}
    virtual void visit(PathExpr& node) {}
    virtual void visit(BinaryExpr& node) {}
    virtual void visit(UnaryExpr& node) {}
    virtual void visit(CallExpr& node) {}
    virtual void visit(MethodCallExpr& node) {}
    virtual void visit(MemberAccessExpr& node) {}
    virtual void visit(IndexExpr& node) {}
    virtual void visit(CastExpr& node) {}
    virtual void visit(BlockExpr& node) {}
    virtual void visit(IfExpr& node) {}
    virtual void visit(MatchExpr& node) {}
    virtual void visit(ClosureExpr& node) {}
    virtual void visit(ConstructExpr& node) {}
    virtual void visit(TupleExpr& node) {}
    virtual void visit(ArrayExpr& node) {}
    virtual void visit(RangeExpr& node) {}
    virtual void visit(RefExpr& node) {}
    virtual void visit(MutRefExpr& node) {}
    virtual void visit(MoveExpr& node) {}
    virtual void visit(AwaitExpr& node) {}
    virtual void visit(TryExpr& node) {}
    virtual void visit(AssignExpr& node) {}
    virtual void visit(CompoundAssignExpr& node) {}
    virtual void visit(StructLiteralExpr& node) {}

    // ---- Patterns ----
    virtual void visit(WildcardPattern& node) {}
    virtual void visit(IdentPattern& node) {}
    virtual void visit(LiteralPattern& node) {}
    virtual void visit(TuplePattern& node) {}
    virtual void visit(ConstructorPattern& node) {}
    virtual void visit(OrPattern& node) {}

    // ---- Types ----
    virtual void visit(NamedType& node) {}
    virtual void visit(GenericType& node) {}
    virtual void visit(ReferenceType& node) {}
    virtual void visit(MutRefType& node) {}
    virtual void visit(TupleType& node) {}
    virtual void visit(FunctionType& node) {}
    virtual void visit(ArrayType& node) {}

    // ---- Dispatch helpers ----
    void visitDecl(Decl& decl);
    void visitStmt(Stmt& stmt);
    void visitExpr(Expr& expr);
    void visitPattern(Pattern& pattern);
    void visitType(TypeNode& type);
};

} // namespace ast
} // namespace flux
