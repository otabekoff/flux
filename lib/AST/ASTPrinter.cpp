#include "flux/AST/AST.h"
#include "flux/AST/Decl.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Stmt.h"
#include "flux/AST/Type.h"
#include "flux/AST/Pattern.h"

#include <iostream>
#include <sstream>
#include <string>

namespace flux {
namespace ast {

/// ASTPrinter: prints a human-readable representation of the AST.
/// Useful for debugging and testing the parser.
class ASTPrinter {
public:
    std::string print(const Module& module);

    std::string printDecl(const Decl& decl, int indent = 0);
    std::string printStmt(const Stmt& stmt, int indent = 0);
    std::string printExpr(const Expr& expr, int indent = 0);
    std::string printType(const TypeNode& type);
    std::string printPattern(const Pattern& pattern);

private:
    std::string indent(int level) const;
};

} // namespace ast
} // namespace flux
