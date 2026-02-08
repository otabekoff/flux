#include "flux/Sema/TypeChecker.h"

#include <cassert>

namespace flux {

TypeChecker::TypeChecker(DiagnosticEngine& diag, const Scope& scope)
    : diag_(diag), scope_(scope) {
    registerBuiltinTypes();
}

void TypeChecker::registerBuiltinTypes() {
    // Primitive types from the Flux spec
    knownTypes_.insert("Int8");
    knownTypes_.insert("Int16");
    knownTypes_.insert("Int32");
    knownTypes_.insert("Int64");
    knownTypes_.insert("UInt8");
    knownTypes_.insert("UInt16");
    knownTypes_.insert("UInt32");
    knownTypes_.insert("UInt64");
    knownTypes_.insert("Float32");
    knownTypes_.insert("Float64");
    knownTypes_.insert("Bool");
    knownTypes_.insert("Char");
    knownTypes_.insert("String");
    knownTypes_.insert("Void");

    // Common standard library types
    knownTypes_.insert("Option");
    knownTypes_.insert("Result");
    knownTypes_.insert("Vec");
    knownTypes_.insert("Map");
    knownTypes_.insert("Set");
    knownTypes_.insert("Box");
    knownTypes_.insert("Rc");
    knownTypes_.insert("Arc");
    knownTypes_.insert("Mutex");
    knownTypes_.insert("Channel");
    knownTypes_.insert("Future");
}

void TypeChecker::check(ast::Module& module) {
    // Register user-defined types from the scope
    for (auto& [name, sym] : scope_.symbols) {
        if (sym.kind == Symbol::Kind::Struct ||
            sym.kind == Symbol::Kind::Class ||
            sym.kind == Symbol::Kind::Enum ||
            sym.kind == Symbol::Kind::Trait ||
            sym.kind == Symbol::Kind::TypeAlias) {
            knownTypes_.insert(name);
        }
    }

    for (auto& decl : module.declarations) {
        checkDecl(*decl);
    }
}

void TypeChecker::checkDecl(ast::Decl& decl) {
    switch (decl.kind) {
    case ast::Decl::Kind::Func:
        checkFuncDecl(static_cast<ast::FuncDecl&>(decl));
        break;
    case ast::Decl::Kind::Struct:
        checkStructDecl(static_cast<ast::StructDecl&>(decl));
        break;
    case ast::Decl::Kind::Class:
        checkClassDecl(static_cast<ast::ClassDecl&>(decl));
        break;
    case ast::Decl::Kind::Enum:
        checkEnumDecl(static_cast<ast::EnumDecl&>(decl));
        break;
    case ast::Decl::Kind::Trait:
        checkTraitDecl(static_cast<ast::TraitDecl&>(decl));
        break;
    case ast::Decl::Kind::Impl:
        checkImplDecl(static_cast<ast::ImplDecl&>(decl));
        break;
    default:
        break;
    }
}

void TypeChecker::checkFuncDecl(ast::FuncDecl& decl) {
    // Validate return type
    if (decl.returnType) {
        std::string retType = typeToString(*decl.returnType);
        if (!isValidType(retType)) {
            diag_.emitError(decl.location,
                            "unknown return type '" + retType + "' in function '" +
                            decl.name + "'");
        }
        currentReturnType_ = retType;
    } else {
        currentReturnType_ = "Void";
    }

    // Validate parameter types
    for (auto& param : decl.params) {
        if (param.type) {
            std::string paramType = typeToString(*param.type);
            if (!isValidType(paramType)) {
                diag_.emitError(decl.location,
                                "unknown parameter type '" + paramType +
                                "' for parameter '" + param.name + "'");
            }
        } else {
            diag_.emitError(decl.location,
                            "parameter '" + param.name +
                            "' must have an explicit type annotation");
        }
    }

    // Check body
    if (decl.body) {
        for (auto& stmt : decl.body->statements) {
            checkStmt(*stmt);
        }
    }

    currentReturnType_.clear();
}

void TypeChecker::checkStructDecl(ast::StructDecl& decl) {
    for (auto& field : decl.fields) {
        if (field.type) {
            std::string fieldType = typeToString(*field.type);
            if (!isValidType(fieldType)) {
                diag_.emitError(decl.location,
                                "unknown field type '" + fieldType +
                                "' for field '" + field.name + "' in struct '" +
                                decl.name + "'");
            }
        }
    }
}

void TypeChecker::checkClassDecl(ast::ClassDecl& decl) {
    for (auto& field : decl.fields) {
        if (field.type) {
            std::string fieldType = typeToString(*field.type);
            if (!isValidType(fieldType)) {
                diag_.emitError(decl.location,
                                "unknown field type '" + fieldType +
                                "' for field '" + field.name + "' in class '" +
                                decl.name + "'");
            }
        }
    }
    for (auto& method : decl.methods) {
        checkFuncDecl(*method);
    }
}

void TypeChecker::checkEnumDecl(ast::EnumDecl& decl) {
    for (auto& variant : decl.variants) {
        // Check tuple field types
        for (auto& fieldType : variant.tupleFields) {
            if (fieldType) {
                std::string ft = typeToString(*fieldType);
                if (!isValidType(ft)) {
                    diag_.emitError(decl.location,
                                    "unknown type '" + ft +
                                    "' in enum variant '" + variant.name + "'");
                }
            }
        }
        // Check struct field types
        for (auto& field : variant.structFields) {
            if (field.type) {
                std::string fieldType = typeToString(*field.type);
                if (!isValidType(fieldType)) {
                    diag_.emitError(decl.location,
                                    "unknown type '" + fieldType +
                                    "' in enum variant '" + variant.name + "'");
                }
            }
        }
    }
}

void TypeChecker::checkTraitDecl(ast::TraitDecl& decl) {
    for (auto& method : decl.methods) {
        checkFuncDecl(*method);
    }
}

void TypeChecker::checkImplDecl(ast::ImplDecl& decl) {
    for (auto& method : decl.methods) {
        checkFuncDecl(*method);
    }
}

// -----------------------------------------------------------------------
// Statement checking
// -----------------------------------------------------------------------

void TypeChecker::checkStmt(ast::Stmt& stmt) {
    switch (stmt.kind) {
    case ast::Stmt::Kind::Let:
        checkLetStmt(static_cast<ast::LetStmt&>(stmt));
        break;
    case ast::Stmt::Kind::Return:
        checkReturnStmt(static_cast<ast::ReturnStmt&>(stmt));
        break;
    case ast::Stmt::Kind::If:
        checkIfStmt(static_cast<ast::IfStmt&>(stmt));
        break;
    case ast::Stmt::Kind::For:
        checkForStmt(static_cast<ast::ForStmt&>(stmt));
        break;
    case ast::Stmt::Kind::While:
        checkWhileStmt(static_cast<ast::WhileStmt&>(stmt));
        break;
    case ast::Stmt::Kind::Block:
        checkBlockStmt(static_cast<ast::BlockStmt&>(stmt));
        break;
    case ast::Stmt::Kind::Expr: {
        auto& es = static_cast<ast::ExprStmt&>(stmt);
        checkExpr(*es.expression);
        break;
    }
    default:
        break;
    }
}

void TypeChecker::checkLetStmt(ast::LetStmt& stmt) {
    // Flux requires explicit type annotations
    if (stmt.type) {
        std::string declType = typeToString(*stmt.type);
        if (!isValidType(declType)) {
            diag_.emitError(stmt.location,
                            "unknown type '" + declType + "' in let binding");
        }
        if (stmt.initializer) {
            std::string initType = checkExpr(*stmt.initializer);
            if (!initType.empty() && !typesCompatible(declType, initType)) {
                diag_.emitError(stmt.location,
                                "type mismatch: expected '" + declType +
                                "', got '" + initType + "'");
            }
        }
    } else {
        diag_.emitError(stmt.location,
                        "variable '" + stmt.name +
                        "' must have an explicit type annotation");
    }
}

void TypeChecker::checkReturnStmt(ast::ReturnStmt& stmt) {
    if (stmt.value) {
        std::string retType = checkExpr(*stmt.value);
        if (!currentReturnType_.empty() && !retType.empty() &&
            !typesCompatible(currentReturnType_, retType)) {
            diag_.emitError(stmt.location,
                            "return type mismatch: expected '" +
                            currentReturnType_ + "', got '" + retType + "'");
        }
    } else {
        if (!currentReturnType_.empty() && currentReturnType_ != "Void") {
            diag_.emitError(stmt.location,
                            "non-void function must return a value");
        }
    }
}

void TypeChecker::checkIfStmt(ast::IfStmt& stmt) {
    std::string condType = checkExpr(*stmt.condition);
    if (!condType.empty() && condType != "Bool") {
        diag_.emitError(stmt.location,
                        "condition must be of type 'Bool', got '" +
                        condType + "'");
    }
    checkStmt(*stmt.thenBranch);
    if (stmt.elseBranch) {
        checkStmt(*stmt.elseBranch);
    }
}

void TypeChecker::checkForStmt(ast::ForStmt& stmt) {
    checkExpr(*stmt.iterable);
    checkStmt(*stmt.body);
}

void TypeChecker::checkWhileStmt(ast::WhileStmt& stmt) {
    std::string condType = checkExpr(*stmt.condition);
    if (!condType.empty() && condType != "Bool") {
        diag_.emitError(stmt.location,
                        "condition must be of type 'Bool', got '" +
                        condType + "'");
    }
    checkStmt(*stmt.body);
}

void TypeChecker::checkBlockStmt(ast::BlockStmt& stmt) {
    for (auto& s : stmt.statements) {
        checkStmt(*s);
    }
}

// -----------------------------------------------------------------------
// Expression checking
// -----------------------------------------------------------------------

std::string TypeChecker::checkExpr(ast::Expr& expr) {
    switch (expr.kind) {
    case ast::Expr::Kind::IntLiteral:
        return "Int64";
    case ast::Expr::Kind::FloatLiteral:
        return "Float64";
    case ast::Expr::Kind::StringLiteral:
        return "String";
    case ast::Expr::Kind::CharLiteral:
        return "Char";
    case ast::Expr::Kind::BoolLiteral:
        return "Bool";
    case ast::Expr::Kind::Ident:
        return checkIdentExpr(static_cast<ast::IdentExpr&>(expr));
    case ast::Expr::Kind::Binary:
        return checkBinaryExpr(static_cast<ast::BinaryExpr&>(expr));
    case ast::Expr::Kind::Call:
        return checkCallExpr(static_cast<ast::CallExpr&>(expr));
    default:
        return ""; // Unknown for now
    }
}

std::string TypeChecker::checkBinaryExpr(ast::BinaryExpr& expr) {
    std::string lhsType = checkExpr(*expr.lhs);
    std::string rhsType = checkExpr(*expr.rhs);

    // Comparison and logical ops return Bool
    switch (expr.op) {
    case ast::BinaryOp::Equal:
    case ast::BinaryOp::NotEqual:
    case ast::BinaryOp::Less:
    case ast::BinaryOp::LessEqual:
    case ast::BinaryOp::Greater:
    case ast::BinaryOp::GreaterEqual:
    case ast::BinaryOp::And:
    case ast::BinaryOp::Or:
        return "Bool";
    default:
        break;
    }

    // Arithmetic ops — both sides should match
    if (!lhsType.empty() && !rhsType.empty() &&
        !typesCompatible(lhsType, rhsType)) {
        diag_.emitError(expr.location,
                        "binary expression type mismatch: '" + lhsType +
                        "' vs '" + rhsType + "'");
    }
    return lhsType.empty() ? rhsType : lhsType;
}

std::string TypeChecker::checkCallExpr(ast::CallExpr& expr) {
    checkExpr(*expr.callee);
    for (auto& arg : expr.arguments) {
        checkExpr(*arg);
    }
    // Full overload resolution would go here
    return "";
}

std::string TypeChecker::checkIdentExpr(ast::IdentExpr& expr) {
    auto* sym = scope_.lookup(expr.name);
    if (sym && !sym->typeName.empty()) {
        return sym->typeName;
    }
    return "";
}

// -----------------------------------------------------------------------
// Type utilities
// -----------------------------------------------------------------------

bool TypeChecker::isValidType(const std::string& typeName) const {
    return knownTypes_.count(typeName) > 0;
}

bool TypeChecker::typesCompatible(const std::string& expected,
                                  const std::string& actual) const {
    if (expected == actual) return true;

    // Integer literal type (Int64) is compatible with any integer type
    if (actual == "Int64" &&
        (expected == "Int8" || expected == "Int16" || expected == "Int32" ||
         expected == "UInt8" || expected == "UInt16" || expected == "UInt32" ||
         expected == "UInt64")) {
        return true;
    }

    // Float literal type (Float64) is compatible with Float32
    if (actual == "Float64" && expected == "Float32") {
        return true;
    }

    return false;
}

std::string TypeChecker::typeToString(const ast::TypeNode& type) const {
    switch (type.kind) {
    case ast::TypeNode::Kind::Named: {
        auto& nt = static_cast<const ast::NamedType&>(type);
        std::string result;
        for (size_t i = 0; i < nt.path.size(); ++i) {
            if (i > 0) result += "::";
            result += nt.path[i];
        }
        return result;
    }
    case ast::TypeNode::Kind::Generic: {
        auto& gt = static_cast<const ast::GenericType&>(type);
        return typeToString(*gt.base);
    }
    case ast::TypeNode::Kind::Reference:
        return "&" + typeToString(*static_cast<const ast::ReferenceType&>(type).inner);
    case ast::TypeNode::Kind::MutRef:
        return "&mut " + typeToString(*static_cast<const ast::MutRefType&>(type).inner);
    case ast::TypeNode::Kind::Array:
        return "[" + typeToString(*static_cast<const ast::ArrayType&>(type).elementType) + "]";
    case ast::TypeNode::Kind::Option:
        return "Option";
    case ast::TypeNode::Kind::Result:
        return "Result";
    case ast::TypeNode::Kind::Tuple:
        return "(tuple)";
    case ast::TypeNode::Kind::Function:
        return "(func)";
    default:
        return "<unknown>";
    }
}

} // namespace flux
