#include "flux/AST/ASTVisitor.h"

namespace flux {
namespace ast {

void ASTVisitor::visitDecl(Decl& decl) {
    switch (decl.kind) {
        case Decl::Kind::Module:    visit(static_cast<ModuleDecl&>(decl)); break;
        case Decl::Kind::Import:    visit(static_cast<ImportDecl&>(decl)); break;
        case Decl::Kind::Func:      visit(static_cast<FuncDecl&>(decl)); break;
        case Decl::Kind::Struct:    visit(static_cast<StructDecl&>(decl)); break;
        case Decl::Kind::Class:     visit(static_cast<ClassDecl&>(decl)); break;
        case Decl::Kind::Enum:      visit(static_cast<EnumDecl&>(decl)); break;
        case Decl::Kind::Trait:     visit(static_cast<TraitDecl&>(decl)); break;
        case Decl::Kind::Impl:      visit(static_cast<ImplDecl&>(decl)); break;
        case Decl::Kind::TypeAlias: visit(static_cast<TypeAliasDecl&>(decl)); break;
    }
}

void ASTVisitor::visitStmt(Stmt& stmt) {
    switch (stmt.kind) {
        case Stmt::Kind::Let:        visit(static_cast<LetStmt&>(stmt)); break;
        case Stmt::Kind::Const:      visit(static_cast<ConstStmt&>(stmt)); break;
        case Stmt::Kind::Return:     visit(static_cast<ReturnStmt&>(stmt)); break;
        case Stmt::Kind::If:         visit(static_cast<IfStmt&>(stmt)); break;
        case Stmt::Kind::Match:      visit(static_cast<MatchStmt&>(stmt)); break;
        case Stmt::Kind::For:        visit(static_cast<ForStmt&>(stmt)); break;
        case Stmt::Kind::While:      visit(static_cast<WhileStmt&>(stmt)); break;
        case Stmt::Kind::Loop:       visit(static_cast<LoopStmt&>(stmt)); break;
        case Stmt::Kind::Break:      visit(static_cast<BreakStmt&>(stmt)); break;
        case Stmt::Kind::Continue:   visit(static_cast<ContinueStmt&>(stmt)); break;
        case Stmt::Kind::Block:      visit(static_cast<BlockStmt&>(stmt)); break;
        case Stmt::Kind::Expr:       visit(static_cast<ExprStmt&>(stmt)); break;
        case Stmt::Kind::Assignment: break; // handled via ExprStmt with AssignExpr
    }
}

void ASTVisitor::visitExpr(Expr& expr) {
    switch (expr.kind) {
        case Expr::Kind::IntLiteral:     visit(static_cast<IntLiteralExpr&>(expr)); break;
        case Expr::Kind::FloatLiteral:   visit(static_cast<FloatLiteralExpr&>(expr)); break;
        case Expr::Kind::StringLiteral:  visit(static_cast<StringLiteralExpr&>(expr)); break;
        case Expr::Kind::CharLiteral:    visit(static_cast<CharLiteralExpr&>(expr)); break;
        case Expr::Kind::BoolLiteral:    visit(static_cast<BoolLiteralExpr&>(expr)); break;
        case Expr::Kind::Ident:          visit(static_cast<IdentExpr&>(expr)); break;
        case Expr::Kind::Path:           visit(static_cast<PathExpr&>(expr)); break;
        case Expr::Kind::Binary:         visit(static_cast<BinaryExpr&>(expr)); break;
        case Expr::Kind::Unary:          visit(static_cast<UnaryExpr&>(expr)); break;
        case Expr::Kind::Call:           visit(static_cast<CallExpr&>(expr)); break;
        case Expr::Kind::MethodCall:     visit(static_cast<MethodCallExpr&>(expr)); break;
        case Expr::Kind::MemberAccess:   visit(static_cast<MemberAccessExpr&>(expr)); break;
        case Expr::Kind::Index:          visit(static_cast<IndexExpr&>(expr)); break;
        case Expr::Kind::Cast:           visit(static_cast<CastExpr&>(expr)); break;
        case Expr::Kind::Block:          visit(static_cast<BlockExpr&>(expr)); break;
        case Expr::Kind::If:             visit(static_cast<IfExpr&>(expr)); break;
        case Expr::Kind::Match:          visit(static_cast<MatchExpr&>(expr)); break;
        case Expr::Kind::Closure:        visit(static_cast<ClosureExpr&>(expr)); break;
        case Expr::Kind::Construct:      visit(static_cast<ConstructExpr&>(expr)); break;
        case Expr::Kind::Tuple:          visit(static_cast<TupleExpr&>(expr)); break;
        case Expr::Kind::Array:          visit(static_cast<ArrayExpr&>(expr)); break;
        case Expr::Kind::Range:          visit(static_cast<RangeExpr&>(expr)); break;
        case Expr::Kind::Ref:            visit(static_cast<RefExpr&>(expr)); break;
        case Expr::Kind::MutRef:         visit(static_cast<MutRefExpr&>(expr)); break;
        case Expr::Kind::Move:           visit(static_cast<MoveExpr&>(expr)); break;
        case Expr::Kind::Await:          visit(static_cast<AwaitExpr&>(expr)); break;
        case Expr::Kind::Try:            visit(static_cast<TryExpr&>(expr)); break;
        case Expr::Kind::Assign:         visit(static_cast<AssignExpr&>(expr)); break;
        case Expr::Kind::CompoundAssign: visit(static_cast<CompoundAssignExpr&>(expr)); break;
        case Expr::Kind::StructLiteral: visit(static_cast<StructLiteralExpr&>(expr)); break;
    }
}

void ASTVisitor::visitPattern(Pattern& pattern) {
    switch (pattern.kind) {
        case Pattern::Kind::Wildcard:    visit(static_cast<WildcardPattern&>(pattern)); break;
        case Pattern::Kind::Identifier:  visit(static_cast<IdentPattern&>(pattern)); break;
        case Pattern::Kind::Literal:     visit(static_cast<LiteralPattern&>(pattern)); break;
        case Pattern::Kind::Tuple:       visit(static_cast<TuplePattern&>(pattern)); break;
        case Pattern::Kind::Constructor: visit(static_cast<ConstructorPattern&>(pattern)); break;
        case Pattern::Kind::Or:          visit(static_cast<OrPattern&>(pattern)); break;
    }
}

void ASTVisitor::visitType(TypeNode& type) {
    switch (type.kind) {
        case TypeNode::Kind::Named:     visit(static_cast<NamedType&>(type)); break;
        case TypeNode::Kind::Generic:   visit(static_cast<GenericType&>(type)); break;
        case TypeNode::Kind::Reference: visit(static_cast<ReferenceType&>(type)); break;
        case TypeNode::Kind::MutRef:    visit(static_cast<MutRefType&>(type)); break;
        case TypeNode::Kind::Tuple:     visit(static_cast<TupleType&>(type)); break;
        case TypeNode::Kind::Function:  visit(static_cast<FunctionType&>(type)); break;
        case TypeNode::Kind::Array:     visit(static_cast<ArrayType&>(type)); break;
        case TypeNode::Kind::Option:    break; // handled as GenericType
        case TypeNode::Kind::Result:    break; // handled as GenericType
        case TypeNode::Kind::Inferred:  break; // should not appear in Flux
    }
}

} // namespace ast
} // namespace flux
