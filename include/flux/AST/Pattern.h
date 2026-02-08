#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Type.h"

#include <string>
#include <vector>

namespace flux {
namespace ast {

// ============================================================================
// Pattern AST Nodes (for match expressions)
// ============================================================================

/// Base class for patterns used in match arms.
class Pattern : public ASTNode {
public:
    enum class Kind {
        Wildcard,       // _
        Identifier,     // name (binds to a variable)
        Literal,        // 42, "hello", true
        Tuple,          // (a, b, c)
        Constructor,    // Option::Some(value), Message::Move { x, y }
        Or,             // pattern1 | pattern2
    };

    Kind kind;
    virtual ~Pattern() = default;

protected:
    explicit Pattern(Kind k, SourceLocation loc = {})
        : ASTNode(loc), kind(k) {}
};

/// Wildcard pattern: _
class WildcardPattern : public Pattern {
public:
    explicit WildcardPattern(SourceLocation loc = {})
        : Pattern(Kind::Wildcard, loc) {}
};

/// Identifier pattern: binds the matched value to a name
class IdentPattern : public Pattern {
public:
    std::string name;
    bool isMutable = false;

    explicit IdentPattern(std::string name, SourceLocation loc = {})
        : Pattern(Kind::Identifier, loc), name(std::move(name)) {}
};

/// Literal pattern: matches a specific literal value
class LiteralPattern : public Pattern {
public:
    ExprPtr literal; // IntLiteralExpr, StringLiteralExpr, BoolLiteralExpr, etc.

    explicit LiteralPattern(ExprPtr literal, SourceLocation loc = {})
        : Pattern(Kind::Literal, loc), literal(std::move(literal)) {}
};

/// Tuple pattern: (a, b, c)
class TuplePattern : public Pattern {
public:
    std::vector<PatternPtr> elements;

    explicit TuplePattern(std::vector<PatternPtr> elements, SourceLocation loc = {})
        : Pattern(Kind::Tuple, loc), elements(std::move(elements)) {}
};

/// Constructor pattern: Option::Some(value), Message::Move { x, y }
class ConstructorPattern : public Pattern {
public:
    std::vector<std::string> path;  // ["Option", "Some"] or ["Message", "Move"]

    // Positional fields: Option::Some(value)
    std::vector<PatternPtr> positionalFields;

    // Named fields: Message::Move { x, y }
    struct NamedField {
        std::string name;
        PatternPtr pattern;
    };
    std::vector<NamedField> namedFields;

    explicit ConstructorPattern(std::vector<std::string> path,
                                SourceLocation loc = {})
        : Pattern(Kind::Constructor, loc), path(std::move(path)) {}
};

/// Or pattern: pattern1 | pattern2
class OrPattern : public Pattern {
public:
    std::vector<PatternPtr> alternatives;

    explicit OrPattern(std::vector<PatternPtr> alternatives,
                       SourceLocation loc = {})
        : Pattern(Kind::Or, loc), alternatives(std::move(alternatives)) {}
};

} // namespace ast
} // namespace flux
