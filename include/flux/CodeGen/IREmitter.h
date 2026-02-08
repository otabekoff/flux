#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Decl.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Stmt.h"
#include "flux/CodeGen/TypeMapper.h"
#include "flux/Common/Diagnostics.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace flux {

/// Emits LLVM IR from Flux AST nodes.
///
/// This is the core code generation class. It traverses the AST and
/// produces LLVM IR using the LLVM IRBuilder API.
class IREmitter {
public:
    IREmitter(llvm::LLVMContext& ctx, llvm::Module& module,
              DiagnosticEngine& diag);

    /// Declare prototypes for a top-level declaration (first pass).
    void declareDecl(ast::Decl& decl);
    void emitDecl(ast::Decl& decl);
    void emitStmt(ast::Stmt& stmt);
    llvm::Value* emitExpr(ast::Expr& expr);

private:
    // Declaration emission
    void declareFunc(ast::FuncDecl& decl);
    void declareStruct(ast::StructDecl& decl);
    void emitFuncDecl(ast::FuncDecl& decl);
    void emitStructDecl(ast::StructDecl& decl);
    void emitEnumDecl(ast::EnumDecl& decl);

    // Statement emission
    void emitLetStmt(ast::LetStmt& stmt);
    void emitReturnStmt(ast::ReturnStmt& stmt);
    void emitIfStmt(ast::IfStmt& stmt);
    void emitForStmt(ast::ForStmt& stmt);
    void emitWhileStmt(ast::WhileStmt& stmt);
    void emitLoopStmt(ast::LoopStmt& stmt);
    void emitBlockStmt(ast::BlockStmt& stmt);
    void emitExprStmt(ast::ExprStmt& stmt);

    // Expression emission
    llvm::Value* emitIntLiteral(ast::IntLiteralExpr& expr);
    llvm::Value* emitFloatLiteral(ast::FloatLiteralExpr& expr);
    llvm::Value* emitStringLiteral(ast::StringLiteralExpr& expr);
    llvm::Value* emitBoolLiteral(ast::BoolLiteralExpr& expr);
    llvm::Value* emitIdent(ast::IdentExpr& expr);
    llvm::Value* emitBinaryExpr(ast::BinaryExpr& expr);
    llvm::Value* emitUnaryExpr(ast::UnaryExpr& expr);
    llvm::Value* emitCallExpr(ast::CallExpr& expr);
    llvm::Value* emitIfExpr(ast::IfExpr& expr);
    llvm::Value* emitBlockExpr(ast::BlockExpr& expr);
    llvm::Value* emitAssignExpr(ast::AssignExpr& expr);

    // Helper to create an alloca in the entry block of a function
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* func,
                                              const std::string& name,
                                              llvm::Type* type);

    llvm::LLVMContext& ctx_;
    llvm::Module& module_;
    llvm::IRBuilder<> builder_;
    DiagnosticEngine& diag_;
    TypeMapper typeMapper_;

    // Named values in the current scope (name -> alloca)
    std::unordered_map<std::string, llvm::AllocaInst*> namedValues_;

    // Stack of break/continue targets for loops
    struct LoopContext {
        llvm::BasicBlock* breakBlock;
        llvm::BasicBlock* continueBlock;
    };
    std::vector<LoopContext> loopStack_;
};

} // namespace flux
