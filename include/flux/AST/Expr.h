#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Pattern.h"
#include "flux/AST/Type.h"


#include <optional>
#include <string>
#include <vector>

namespace flux {
namespace ast {

// ============================================================================
// Expression AST Nodes
// ============================================================================

/// Base class for all expressions.
class Expr : public ASTNode {
public:
  enum class Kind {
    IntLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,
    BoolLiteral,
    Ident,
    Path, // std::io::println
    Binary,
    Unary,
    Call,
    MethodCall,
    MemberAccess,
    Index,
    Cast,
    Block,
    If,
    Match,
    Closure,
    Construct,     // Point { x: 1.0, y: 2.0 }
    StructLiteral, // Sugar for struct construction by name
    Tuple,
    Array,
    Range,
    Ref,    // ref expr
    MutRef, // mut ref expr
    Move,   // move expr
    Await,  // await expr
    Try,    // expr?
    Assign,
    CompoundAssign,
  };

  Kind kind;
  virtual ~Expr() = default;

protected:
  explicit Expr(Kind k, SourceLocation loc = {}) : ASTNode(loc), kind(k) {}
};

// --- Literal expressions ---

class IntLiteralExpr : public Expr {
public:
  int64_t value;

  explicit IntLiteralExpr(int64_t val, SourceLocation loc = {})
      : Expr(Kind::IntLiteral, loc), value(val) {}
};

class FloatLiteralExpr : public Expr {
public:
  double value;

  explicit FloatLiteralExpr(double val, SourceLocation loc = {})
      : Expr(Kind::FloatLiteral, loc), value(val) {}
};

class StringLiteralExpr : public Expr {
public:
  std::string value;

  explicit StringLiteralExpr(std::string val, SourceLocation loc = {})
      : Expr(Kind::StringLiteral, loc), value(std::move(val)) {}
};

class CharLiteralExpr : public Expr {
public:
  char32_t value;

  explicit CharLiteralExpr(char32_t val, SourceLocation loc = {})
      : Expr(Kind::CharLiteral, loc), value(val) {}
};

class BoolLiteralExpr : public Expr {
public:
  bool value;

  explicit BoolLiteralExpr(bool val, SourceLocation loc = {})
      : Expr(Kind::BoolLiteral, loc), value(val) {}
};

// --- Identifier & Path ---

class IdentExpr : public Expr {
public:
  std::string name;

  explicit IdentExpr(std::string name, SourceLocation loc = {})
      : Expr(Kind::Ident, loc), name(std::move(name)) {}
};

/// A path expression like std::io::println or Option::Some
class PathExpr : public Expr {
public:
  std::vector<std::string> segments;

  explicit PathExpr(std::vector<std::string> segments, SourceLocation loc = {})
      : Expr(Kind::Path, loc), segments(std::move(segments)) {}
};

// --- Operators ---

enum class BinaryOp {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Equal,
  NotEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  And,
  Or,
  BitAnd,
  BitOr,
  BitXor,
  ShiftLeft,
  ShiftRight,
  // String concatenation uses Add
};

enum class UnaryOp {
  Negate,     // -
  Not,        // not
  BitwiseNot, // ~
};

class BinaryExpr : public Expr {
public:
  BinaryOp op;
  ExprPtr lhs;
  ExprPtr rhs;

  BinaryExpr(BinaryOp op, ExprPtr left, ExprPtr right, SourceLocation loc = {})
      : Expr(Kind::Binary, loc), op(op), lhs(std::move(left)),
        rhs(std::move(right)) {}
};

class UnaryExpr : public Expr {
public:
  UnaryOp op;
  ExprPtr operand;

  UnaryExpr(UnaryOp op, ExprPtr operand, SourceLocation loc = {})
      : Expr(Kind::Unary, loc), op(op), operand(std::move(operand)) {}
};

// --- Call expressions ---

class CallExpr : public Expr {
public:
  ExprPtr callee; // function expression
  ExprList arguments;

  CallExpr(ExprPtr callee, ExprList args, SourceLocation loc = {})
      : Expr(Kind::Call, loc), callee(std::move(callee)),
        arguments(std::move(args)) {}
};

class MethodCallExpr : public Expr {
public:
  ExprPtr object;
  std::string method;
  ExprList arguments;

  MethodCallExpr(ExprPtr receiver, std::string method, ExprList args,
                 SourceLocation loc = {})
      : Expr(Kind::MethodCall, loc), object(std::move(receiver)),
        method(std::move(method)), arguments(std::move(args)) {}
};

// --- Access expressions ---

class MemberAccessExpr : public Expr {
public:
  ExprPtr object;
  std::string member;

  MemberAccessExpr(ExprPtr object, std::string member, SourceLocation loc = {})
      : Expr(Kind::MemberAccess, loc), object(std::move(object)),
        member(std::move(member)) {}
};

class IndexExpr : public Expr {
public:
  ExprPtr object;
  ExprPtr index;

  IndexExpr(ExprPtr object, ExprPtr index, SourceLocation loc = {})
      : Expr(Kind::Index, loc), object(std::move(object)),
        index(std::move(index)) {}
};

// --- Type cast ---

class CastExpr : public Expr {
public:
  ExprPtr expr;
  TypeNodePtr targetType;

  CastExpr(ExprPtr e, TypeNodePtr type, SourceLocation loc = {})
      : Expr(Kind::Cast, loc), expr(std::move(e)), targetType(std::move(type)) {
  }
};

// --- Block expression ---

class BlockExpr : public Expr {
public:
  StmtList statements;
  ExprPtr finalExpr; // optional trailing expression (value of the block)

  explicit BlockExpr(StmtList stmts, ExprPtr tail = nullptr,
                     SourceLocation loc = {})
      : Expr(Kind::Block, loc), statements(std::move(stmts)),
        finalExpr(std::move(tail)) {}
};

// --- If expression ---

class IfExpr : public Expr {
public:
  ExprPtr condition;
  ExprPtr thenExpr; // Block
  ExprPtr elseExpr; // Block or another IfExpr (else if), nullable

  IfExpr(ExprPtr cond, ExprPtr thenBr, ExprPtr elseBr = nullptr,
         SourceLocation loc = {})
      : Expr(Kind::If, loc), condition(std::move(cond)),
        thenExpr(std::move(thenBr)), elseExpr(std::move(elseBr)) {}
};

// --- Match expression ---

struct MatchArm {
  PatternPtr pattern;
  ExprPtr guard; // optional guard (if condition)
  ExprPtr body;
  SourceLocation location;
};

class MatchExpr : public Expr {
public:
  ExprPtr scrutinee;
  std::vector<MatchArm> arms;

  MatchExpr(ExprPtr scrutinee, std::vector<MatchArm> arms,
            SourceLocation loc = {})
      : Expr(Kind::Match, loc), scrutinee(std::move(scrutinee)),
        arms(std::move(arms)) {}
};

// --- Closure ---

struct ClosureParam {
  std::string name;
  TypeNodePtr type;
};

class ClosureExpr : public Expr {
public:
  std::vector<ClosureParam> params;
  TypeNodePtr returnType; // optional
  ExprPtr body;
  bool isMoveCapture = false;

  ClosureExpr(std::vector<ClosureParam> params, TypeNodePtr retType,
              ExprPtr body, SourceLocation loc = {})
      : Expr(Kind::Closure, loc), params(std::move(params)),
        returnType(std::move(retType)), body(std::move(body)) {}
};

// --- Struct construction ---

struct FieldInit {
  std::string name;
  ExprPtr value;
  SourceLocation location;
};

class ConstructExpr : public Expr {
public:
  ExprPtr typePath; // The type being constructed (usually a PathExpr)
  std::vector<FieldInit> fields;

  ConstructExpr(ExprPtr typePath, std::vector<FieldInit> fields,
                SourceLocation loc = {})
      : Expr(Kind::Construct, loc), typePath(std::move(typePath)),
        fields(std::move(fields)) {}
};

/// Struct literal expression created by name: `Point { x: 1.0, y: 2.0 }`
class StructLiteralExpr : public Expr {
public:
  std::string typeName;
  std::vector<std::pair<std::string, ExprPtr>> fields;

  StructLiteralExpr(std::string typeName,
                    std::vector<std::pair<std::string, ExprPtr>> fields,
                    SourceLocation loc = {})
      : Expr(Kind::StructLiteral, loc), typeName(std::move(typeName)),
        fields(std::move(fields)) {}
};

// --- Tuple & Array literals ---

class TupleExpr : public Expr {
public:
  ExprList elements;

  explicit TupleExpr(ExprList elements, SourceLocation loc = {})
      : Expr(Kind::Tuple, loc), elements(std::move(elements)) {}
};

class ArrayExpr : public Expr {
public:
  ExprList elements;

  explicit ArrayExpr(ExprList elements, SourceLocation loc = {})
      : Expr(Kind::Array, loc), elements(std::move(elements)) {}
};

// --- Range ---

class RangeExpr : public Expr {
public:
  ExprPtr start;
  ExprPtr end;
  bool inclusive = false; // .. vs ..=

  RangeExpr(ExprPtr start, ExprPtr end, bool inclusive = false,
            SourceLocation loc = {})
      : Expr(Kind::Range, loc), start(std::move(start)), end(std::move(end)),
        inclusive(inclusive) {}
};

// --- Ownership expressions ---

class RefExpr : public Expr {
public:
  ExprPtr operand;

  explicit RefExpr(ExprPtr operand, SourceLocation loc = {})
      : Expr(Kind::Ref, loc), operand(std::move(operand)) {}
};

class MutRefExpr : public Expr {
public:
  ExprPtr operand;

  explicit MutRefExpr(ExprPtr operand, SourceLocation loc = {})
      : Expr(Kind::MutRef, loc), operand(std::move(operand)) {}
};

class MoveExpr : public Expr {
public:
  ExprPtr operand;

  explicit MoveExpr(ExprPtr operand, SourceLocation loc = {})
      : Expr(Kind::Move, loc), operand(std::move(operand)) {}
};

// --- Async/Await ---

class AwaitExpr : public Expr {
public:
  ExprPtr operand;

  explicit AwaitExpr(ExprPtr operand, SourceLocation loc = {})
      : Expr(Kind::Await, loc), operand(std::move(operand)) {}
};

// --- Try (?) operator ---

class TryExpr : public Expr {
public:
  ExprPtr operand;

  explicit TryExpr(ExprPtr operand, SourceLocation loc = {})
      : Expr(Kind::Try, loc), operand(std::move(operand)) {}
};

// --- Assignment ---

class AssignExpr : public Expr {
public:
  ExprPtr target;
  ExprPtr value;

  AssignExpr(ExprPtr target, ExprPtr value, SourceLocation loc = {})
      : Expr(Kind::Assign, loc), target(std::move(target)),
        value(std::move(value)) {}
};

enum class CompoundAssignOp {
  AddAssign,
  SubAssign,
  MulAssign,
  DivAssign,
  ModAssign,
  AndAssign,
  OrAssign,
  XorAssign,
};

class CompoundAssignExpr : public Expr {
public:
  CompoundAssignOp op;
  ExprPtr target;
  ExprPtr value;

  CompoundAssignExpr(CompoundAssignOp op, ExprPtr target, ExprPtr value,
                     SourceLocation loc = {})
      : Expr(Kind::CompoundAssign, loc), op(op), target(std::move(target)),
        value(std::move(value)) {}
};

} // namespace ast
} // namespace flux
