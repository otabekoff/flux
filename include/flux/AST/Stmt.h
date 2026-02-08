#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Type.h"

#include <optional>
#include <string>
#include <vector>

namespace flux {
namespace ast {

// ============================================================================
// Statement AST Nodes
// ============================================================================

/// Base class for all statements.
class Stmt : public ASTNode {
public:
    enum class Kind {
        Let,
        Const,
        Return,
        If,
        Match,
        For,
        While,
        Loop,
        Break,
        Continue,
        Block,
        Expr,
        Assignment,
    };

    Kind kind;
    virtual ~Stmt() = default;

protected:
    explicit Stmt(Kind k, SourceLocation loc = {})
        : ASTNode(loc), kind(k) {}
};

// --- Let statement ---
// let name: Type = value;
// let mut name: Type = value;
class LetStmt : public Stmt {
public:
    std::string name;
    TypeNodePtr type;       // explicit type annotation (always required in Flux)
    ExprPtr initializer;    // optional initializer
    bool isMutable = false; // let vs let mut

    LetStmt(std::string name, TypeNodePtr type, ExprPtr init,
            bool isMutable, SourceLocation loc = {})
        : Stmt(Kind::Let, loc), name(std::move(name)),
          type(std::move(type)), initializer(std::move(init)),
          isMutable(isMutable) {}
};

// --- Const statement ---
// const NAME: Type = value;
class ConstStmt : public Stmt {
public:
    std::string name;
    TypeNodePtr type;
    ExprPtr value; // must be compile-time evaluable

    ConstStmt(std::string name, TypeNodePtr type, ExprPtr value,
              SourceLocation loc = {})
        : Stmt(Kind::Const, loc), name(std::move(name)),
          type(std::move(type)), value(std::move(value)) {}
};

// --- Return statement ---
class ReturnStmt : public Stmt {
public:
    ExprPtr value; // nullable for void returns

    explicit ReturnStmt(ExprPtr value = nullptr, SourceLocation loc = {})
        : Stmt(Kind::Return, loc), value(std::move(value)) {}
};

// --- If statement ---
class IfStmt : public Stmt {
public:
    ExprPtr condition;
    StmtPtr thenBranch; // BlockStmt
    StmtPtr elseBranch; // BlockStmt or another IfStmt (else if), nullable

    IfStmt(ExprPtr cond, StmtPtr thenBr, StmtPtr elseBr = nullptr,
           SourceLocation loc = {})
        : Stmt(Kind::If, loc), condition(std::move(cond)),
          thenBranch(std::move(thenBr)), elseBranch(std::move(elseBr)) {}
};

// --- Match statement ---
class MatchStmt : public Stmt {
public:
    ExprPtr scrutinee;
    std::vector<MatchArm> arms;

    MatchStmt(ExprPtr scrutinee, std::vector<MatchArm> arms,
              SourceLocation loc = {})
        : Stmt(Kind::Match, loc), scrutinee(std::move(scrutinee)),
          arms(std::move(arms)) {}
};

// --- For statement ---
// for name: Type in iterable { body }
class ForStmt : public Stmt {
public:
    std::string varName;
    TypeNodePtr varType;
    ExprPtr iterable;
    StmtPtr body; // BlockStmt

    ForStmt(std::string varName, TypeNodePtr varType, ExprPtr iterable,
            StmtPtr body, SourceLocation loc = {})
        : Stmt(Kind::For, loc), varName(std::move(varName)),
          varType(std::move(varType)), iterable(std::move(iterable)),
          body(std::move(body)) {}
};

// --- While statement ---
class WhileStmt : public Stmt {
public:
    ExprPtr condition;
    StmtPtr body;

    WhileStmt(ExprPtr condition, StmtPtr body, SourceLocation loc = {})
        : Stmt(Kind::While, loc), condition(std::move(condition)),
          body(std::move(body)) {}
};

// --- Loop statement (infinite) ---
class LoopStmt : public Stmt {
public:
    StmtPtr body;

    explicit LoopStmt(StmtPtr body, SourceLocation loc = {})
        : Stmt(Kind::Loop, loc), body(std::move(body)) {}
};

// --- Break & Continue ---
class BreakStmt : public Stmt {
public:
    explicit BreakStmt(SourceLocation loc = {})
        : Stmt(Kind::Break, loc) {}
};

class ContinueStmt : public Stmt {
public:
    explicit ContinueStmt(SourceLocation loc = {})
        : Stmt(Kind::Continue, loc) {}
};

// --- Block statement ---
class BlockStmt : public Stmt {
public:
    StmtList statements;

    explicit BlockStmt(StmtList stmts, SourceLocation loc = {})
        : Stmt(Kind::Block, loc), statements(std::move(stmts)) {}
};

// --- Expression statement ---
class ExprStmt : public Stmt {
public:
    ExprPtr expression;

    explicit ExprStmt(ExprPtr expr, SourceLocation loc = {})
        : Stmt(Kind::Expr, loc), expression(std::move(expr)) {}
};

} // namespace ast
} // namespace flux
