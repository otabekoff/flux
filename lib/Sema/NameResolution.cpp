#include "flux/Sema/NameResolution.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Stmt.h"

#include <cassert>

namespace flux {

// -----------------------------------------------------------------------
// Scope implementation
// -----------------------------------------------------------------------

Symbol* Scope::lookup(const std::string& symbolName) {
    auto it = symbols.find(symbolName);
    if (it != symbols.end()) {
        return &it->second;
    }
    if (parent) {
        return parent->lookup(symbolName);
    }
    return nullptr;
}

const Symbol* Scope::lookup(const std::string& symbolName) const {
    auto it = symbols.find(symbolName);
    if (it != symbols.end()) {
        return &it->second;
    }
    if (parent) {
        return parent->lookup(symbolName);
    }
    return nullptr;
}

bool Scope::insert(const std::string& symbolName, Symbol sym) {
    auto [it, inserted] = symbols.emplace(symbolName, std::move(sym));
    return inserted;
}

// -----------------------------------------------------------------------
// NameResolver implementation
// -----------------------------------------------------------------------

NameResolver::NameResolver(DiagnosticEngine& diag, Scope& globalScope)
    : diag_(diag), currentScope_(&globalScope) {}

void NameResolver::resolve(ast::Module& module) {
    // First pass: register all top-level declarations (forward declarations)
    for (auto& decl : module.declarations) {
        registerDecl(*decl);
    }

    // Second pass: resolve bodies
    for (auto& decl : module.declarations) {
        resolveDecl(*decl);
    }
}

void NameResolver::registerDecl(ast::Decl& decl) {
    Symbol sym;
    sym.location = decl.location;

    switch (decl.kind) {
    case ast::Decl::Kind::Func: {
        auto& fd = static_cast<ast::FuncDecl&>(decl);
        sym.name = fd.name;
        sym.kind = Symbol::Kind::Function;
        sym.visibility = fd.visibility;
        break;
    }
    case ast::Decl::Kind::Struct: {
        auto& sd = static_cast<ast::StructDecl&>(decl);
        sym.name = sd.name;
        sym.kind = Symbol::Kind::Struct;
        sym.visibility = sd.visibility;
        break;
    }
    case ast::Decl::Kind::Class: {
        auto& cd = static_cast<ast::ClassDecl&>(decl);
        sym.name = cd.name;
        sym.kind = Symbol::Kind::Class;
        sym.visibility = cd.visibility;
        break;
    }
    case ast::Decl::Kind::Enum: {
        auto& ed = static_cast<ast::EnumDecl&>(decl);
        sym.name = ed.name;
        sym.kind = Symbol::Kind::Enum;
        sym.visibility = ed.visibility;
        break;
    }
    case ast::Decl::Kind::Trait: {
        auto& td = static_cast<ast::TraitDecl&>(decl);
        sym.name = td.name;
        sym.kind = Symbol::Kind::Trait;
        sym.visibility = td.visibility;
        break;
    }
    case ast::Decl::Kind::TypeAlias: {
        auto& ta = static_cast<ast::TypeAliasDecl&>(decl);
        sym.name = ta.name;
        sym.kind = Symbol::Kind::TypeAlias;
        sym.visibility = ta.visibility;
        break;
    }
    default:
        return; // Module, Import, Impl don't register names directly
    }

    if (!currentScope_->insert(sym.name, sym)) {
        diag_.emitError(decl.location,
                        "redefinition of '" + sym.name + "'");
    }
}

void NameResolver::resolveDecl(ast::Decl& decl) {
    switch (decl.kind) {
    case ast::Decl::Kind::Module:
        // Module declarations just set context
        break;

    case ast::Decl::Kind::Import:
        resolveImport(static_cast<ast::ImportDecl&>(decl));
        break;

    case ast::Decl::Kind::Func:
        resolveFunc(static_cast<ast::FuncDecl&>(decl));
        break;

    case ast::Decl::Kind::Struct:
        resolveStruct(static_cast<ast::StructDecl&>(decl));
        break;

    case ast::Decl::Kind::Class:
        resolveClass(static_cast<ast::ClassDecl&>(decl));
        break;

    case ast::Decl::Kind::Enum:
        resolveEnum(static_cast<ast::EnumDecl&>(decl));
        break;

    case ast::Decl::Kind::Trait:
        resolveTrait(static_cast<ast::TraitDecl&>(decl));
        break;

    case ast::Decl::Kind::Impl:
        resolveImpl(static_cast<ast::ImplDecl&>(decl));
        break;

    case ast::Decl::Kind::TypeAlias:
        resolveTypeAlias(static_cast<ast::TypeAliasDecl&>(decl));
        break;
    }
}

void NameResolver::resolveStmt(ast::Stmt& stmt) {
    switch (stmt.kind) {
    case ast::Stmt::Kind::Let: {
        auto& ls = static_cast<ast::LetStmt&>(stmt);
        // Resolve initialiser first (before adding to scope)
        if (ls.initializer) {
            resolveExpr(*ls.initializer);
        }
        // Register the variable in the current scope
        Symbol sym;
        sym.name = ls.name;
        sym.kind = Symbol::Kind::Variable;
        sym.location = ls.location;
        sym.isMutable = ls.isMutable;
        if (!currentScope_->insert(sym.name, sym)) {
            diag_.emitError(ls.location,
                            "redefinition of variable '" + ls.name + "'");
        }
        break;
    }
    case ast::Stmt::Kind::Const: {
        auto& cs = static_cast<ast::ConstStmt&>(stmt);
        if (cs.value) {
            resolveExpr(*cs.value);
        }
        Symbol sym;
        sym.name = cs.name;
        sym.kind = Symbol::Kind::Variable;
        sym.location = cs.location;
        sym.isMutable = false;
        if (!currentScope_->insert(sym.name, sym)) {
            diag_.emitError(cs.location,
                            "redefinition of constant '" + cs.name + "'");
        }
        break;
    }
    case ast::Stmt::Kind::Return: {
        auto& rs = static_cast<ast::ReturnStmt&>(stmt);
        if (rs.value) {
            resolveExpr(*rs.value);
        }
        break;
    }
    case ast::Stmt::Kind::If: {
        auto& is = static_cast<ast::IfStmt&>(stmt);
        resolveExpr(*is.condition);
        resolveStmt(*is.thenBranch);
        if (is.elseBranch) {
            resolveStmt(*is.elseBranch);
        }
        break;
    }
    case ast::Stmt::Kind::For: {
        auto& fs = static_cast<ast::ForStmt&>(stmt);
        resolveExpr(*fs.iterable);
        enterScope("for");
        Symbol sym;
        sym.name = fs.varName;
        sym.kind = Symbol::Kind::Variable;
        sym.location = fs.location;
        sym.isMutable = false;
        currentScope_->insert(sym.name, sym);
        resolveStmt(*fs.body);
        exitScope();
        break;
    }
    case ast::Stmt::Kind::While: {
        auto& ws = static_cast<ast::WhileStmt&>(stmt);
        resolveExpr(*ws.condition);
        resolveStmt(*ws.body);
        break;
    }
    case ast::Stmt::Kind::Loop: {
        auto& ls = static_cast<ast::LoopStmt&>(stmt);
        resolveStmt(*ls.body);
        break;
    }
    case ast::Stmt::Kind::Block: {
        auto& bs = static_cast<ast::BlockStmt&>(stmt);
        enterScope("block");
        for (auto& s : bs.statements) {
            resolveStmt(*s);
        }
        exitScope();
        break;
    }
    case ast::Stmt::Kind::Expr: {
        auto& es = static_cast<ast::ExprStmt&>(stmt);
        resolveExpr(*es.expression);
        break;
    }
    case ast::Stmt::Kind::Match: {
        auto& ms = static_cast<ast::MatchStmt&>(stmt);
        resolveExpr(*ms.scrutinee);
        // Match arms need pattern scope resolution
        break;
    }
    default:
        break;
    }
}

void NameResolver::resolveExpr(ast::Expr& expr) {
    switch (expr.kind) {
    case ast::Expr::Kind::Ident: {
        auto& ie = static_cast<ast::IdentExpr&>(expr);
        if (!currentScope_->lookup(ie.name)) {
            diag_.emitError(expr.location,
                            "use of undeclared identifier '" + ie.name + "'");
        }
        break;
    }
    case ast::Expr::Kind::Binary: {
        auto& be = static_cast<ast::BinaryExpr&>(expr);
        resolveExpr(*be.lhs);
        resolveExpr(*be.rhs);
        break;
    }
    case ast::Expr::Kind::Unary: {
        auto& ue = static_cast<ast::UnaryExpr&>(expr);
        resolveExpr(*ue.operand);
        break;
    }
    case ast::Expr::Kind::Call: {
        auto& ce = static_cast<ast::CallExpr&>(expr);
        resolveExpr(*ce.callee);
        for (auto& arg : ce.arguments) {
            resolveExpr(*arg);
        }
        break;
    }
    case ast::Expr::Kind::MethodCall: {
        auto& mc = static_cast<ast::MethodCallExpr&>(expr);
        resolveExpr(*mc.object);
        for (auto& arg : mc.arguments) {
            resolveExpr(*arg);
        }
        break;
    }
    case ast::Expr::Kind::MemberAccess: {
        auto& ma = static_cast<ast::MemberAccessExpr&>(expr);
        resolveExpr(*ma.object);
        break;
    }
    case ast::Expr::Kind::Index: {
        auto& ie = static_cast<ast::IndexExpr&>(expr);
        resolveExpr(*ie.object);
        resolveExpr(*ie.index);
        break;
    }
    case ast::Expr::Kind::Block: {
        auto& be = static_cast<ast::BlockExpr&>(expr);
        enterScope("block_expr");
        for (auto& s : be.statements) {
            resolveStmt(*s);
        }
        if (be.finalExpr) {
            resolveExpr(*be.finalExpr);
        }
        exitScope();
        break;
    }
    case ast::Expr::Kind::If: {
        auto& ie = static_cast<ast::IfExpr&>(expr);
        resolveExpr(*ie.condition);
        resolveExpr(*ie.thenExpr);
        if (ie.elseExpr) {
            resolveExpr(*ie.elseExpr);
        }
        break;
    }
    case ast::Expr::Kind::Closure: {
        auto& cl = static_cast<ast::ClosureExpr&>(expr);
        enterScope("closure");
        for (auto& p : cl.params) {
            Symbol sym;
            sym.name = p.name;
            sym.kind = Symbol::Kind::Variable;
            sym.isMutable = false;
            currentScope_->insert(sym.name, sym);
        }
        resolveExpr(*cl.body);
        exitScope();
        break;
    }
    case ast::Expr::Kind::Assign: {
        auto& ae = static_cast<ast::AssignExpr&>(expr);
        resolveExpr(*ae.target);
        resolveExpr(*ae.value);
        break;
    }
    case ast::Expr::Kind::CompoundAssign: {
        auto& ca = static_cast<ast::CompoundAssignExpr&>(expr);
        resolveExpr(*ca.target);
        resolveExpr(*ca.value);
        break;
    }
    case ast::Expr::Kind::Cast: {
        auto& ce = static_cast<ast::CastExpr&>(expr);
        resolveExpr(*ce.expr);
        break;
    }
    case ast::Expr::Kind::Tuple: {
        auto& te = static_cast<ast::TupleExpr&>(expr);
        for (auto& e : te.elements) {
            resolveExpr(*e);
        }
        break;
    }
    case ast::Expr::Kind::Array: {
        auto& ae = static_cast<ast::ArrayExpr&>(expr);
        for (auto& e : ae.elements) {
            resolveExpr(*e);
        }
        break;
    }
    case ast::Expr::Kind::Ref: {
        auto& re = static_cast<ast::RefExpr&>(expr);
        resolveExpr(*re.operand);
        break;
    }
    case ast::Expr::Kind::MutRef: {
        auto& re = static_cast<ast::MutRefExpr&>(expr);
        resolveExpr(*re.operand);
        break;
    }
    case ast::Expr::Kind::Move: {
        auto& me = static_cast<ast::MoveExpr&>(expr);
        resolveExpr(*me.operand);
        break;
    }
    case ast::Expr::Kind::Await: {
        auto& ae = static_cast<ast::AwaitExpr&>(expr);
        resolveExpr(*ae.operand);
        break;
    }
    case ast::Expr::Kind::Try: {
        auto& te = static_cast<ast::TryExpr&>(expr);
        resolveExpr(*te.operand);
        break;
    }
    case ast::Expr::Kind::Range: {
        auto& re = static_cast<ast::RangeExpr&>(expr);
        if (re.start) resolveExpr(*re.start);
        if (re.end) resolveExpr(*re.end);
        break;
    }
    case ast::Expr::Kind::Construct: {
        auto& ce = static_cast<ast::ConstructExpr&>(expr);
        // Resolve the type path expression
        if (ce.typePath) {
            resolveExpr(*ce.typePath);
        }
        for (auto& fi : ce.fields) {
            resolveExpr(*fi.value);
        }
        break;
    }
    case ast::Expr::Kind::Match: {
        auto& me = static_cast<ast::MatchExpr&>(expr);
        resolveExpr(*me.scrutinee);
        // Each arm needs pattern and expression resolution
        for (auto& arm : me.arms) {
            enterScope("match_arm");
            resolveExpr(*arm.body);
            exitScope();
        }
        break;
    }
    default:
        // Literals (int, float, string, char, bool), path — no resolution needed
        break;
    }
}

void NameResolver::enterScope(const std::string& name) {
    auto child = std::make_unique<Scope>(name, currentScope_);
    auto* raw = child.get();
    currentScope_->children.push_back(std::move(child));
    currentScope_ = raw;
}

void NameResolver::exitScope() {
    assert(currentScope_->parent && "Cannot exit the global scope");
    currentScope_ = currentScope_->parent;
}

// -----------------------------------------------------------------------
// Private helpers for specific declarations
// -----------------------------------------------------------------------

void NameResolver::resolveImport(ast::ImportDecl& /*decl*/) {
    // TODO: cross-module import resolution
}

void NameResolver::resolveFunc(ast::FuncDecl& decl) {
    enterScope(decl.name);

    // Register generic parameters
    for (auto& gp : decl.genericParams) {
        Symbol sym;
        sym.name = gp.name;
        sym.kind = Symbol::Kind::GenericParam;
        sym.location = decl.location;
        currentScope_->insert(sym.name, sym);
    }

    // Register parameters
    for (auto& param : decl.params) {
        Symbol sym;
        sym.name = param.name;
        sym.kind = Symbol::Kind::Variable;
        sym.location = decl.location;
        sym.isMutable = false;
        currentScope_->insert(sym.name, sym);
    }

    // Resolve body
    if (decl.body) {
        for (auto& stmt : decl.body->statements) {
            resolveStmt(*stmt);
        }
    }

    exitScope();
}

void NameResolver::resolveStruct(ast::StructDecl& decl) {
    enterScope(decl.name);
    for (auto& gp : decl.genericParams) {
        Symbol sym;
        sym.name = gp.name;
        sym.kind = Symbol::Kind::GenericParam;
        sym.location = decl.location;
        currentScope_->insert(sym.name, sym);
    }
    exitScope();
}

void NameResolver::resolveClass(ast::ClassDecl& decl) {
    enterScope(decl.name);
    for (auto& gp : decl.genericParams) {
        Symbol sym;
        sym.name = gp.name;
        sym.kind = Symbol::Kind::GenericParam;
        sym.location = decl.location;
        currentScope_->insert(sym.name, sym);
    }
    // Resolve methods
    for (auto& method : decl.methods) {
        resolveFunc(*method);
    }
    exitScope();
}

void NameResolver::resolveEnum(ast::EnumDecl& decl) {
    // Register enum variants as symbols in the parent scope
    for (auto& variant : decl.variants) {
        Symbol sym;
        sym.name = decl.name + "::" + variant.name;
        sym.kind = Symbol::Kind::EnumVariant;
        sym.location = decl.location;
        currentScope_->insert(variant.name, sym);
    }
}

void NameResolver::resolveTrait(ast::TraitDecl& decl) {
    enterScope(decl.name);
    for (auto& method : decl.methods) {
        registerDecl(*method);
    }
    exitScope();
}

void NameResolver::resolveImpl(ast::ImplDecl& decl) {
    enterScope("impl");
    for (auto& method : decl.methods) {
        resolveFunc(*method);
    }
    exitScope();
}

void NameResolver::resolveTypeAlias(ast::TypeAliasDecl& /*decl*/) {
    // Type alias underlying type will be checked by TypeChecker
}

} // namespace flux
