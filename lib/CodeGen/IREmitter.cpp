#include "flux/CodeGen/IREmitter.h"
#include "flux/AST/Pattern.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>

namespace flux {

IREmitter::IREmitter(llvm::LLVMContext &ctx, llvm::Module &module,
                     DiagnosticEngine &diag)
    : ctx_(ctx), module_(module), builder_(ctx), diag_(diag), typeMapper_(ctx) {
}

// -----------------------------------------------------------------------
// Declaration emission
// -----------------------------------------------------------------------

void IREmitter::emitDecl(ast::Decl &decl) {
  switch (decl.kind) {
  case ast::Decl::Kind::Func:
    emitFuncDecl(static_cast<ast::FuncDecl &>(decl));
    break;
  case ast::Decl::Kind::Struct:
    emitStructDecl(static_cast<ast::StructDecl &>(decl));
    break;
  case ast::Decl::Kind::Enum:
    emitEnumDecl(static_cast<ast::EnumDecl &>(decl));
    break;
  default:
    // Other declarations (traits, imports, modules, impls)
    // are handled differently or are metadata-only
    break;
  }
}

void IREmitter::emitFuncDecl(ast::FuncDecl &decl) {
  // Determine return type
  llvm::Type *retType = decl.returnType ? typeMapper_.mapType(*decl.returnType)
                                        : typeMapper_.getVoidType();

  // Determine parameter types
  std::vector<llvm::Type *> paramTypes;
  for (auto &param : decl.params) {
    if (param.type) {
      paramTypes.push_back(typeMapper_.mapType(*param.type));
    } else {
      // Should have been caught by type checker
      paramTypes.push_back(llvm::PointerType::getUnqual(ctx_));
    }
  }

  // Create function type
  auto *funcType = llvm::FunctionType::get(retType, paramTypes, false);

  // Set linkage: 'pub' declarations get external linkage
  auto linkage = (decl.visibility == ast::Decl::Visibility::Public)
                     ? llvm::Function::ExternalLinkage
                     : llvm::Function::InternalLinkage;

  // Handle "main" function specially — always external
  if (decl.name == "main") {
    linkage = llvm::Function::ExternalLinkage;
  }

  auto *func = llvm::Function::Create(funcType, linkage, decl.name, &module_);

  // Name arguments
  size_t idx = 0;
  for (auto &arg : func->args()) {
    arg.setName(decl.params[idx].name);
    ++idx;
  }

  // If there's no body, it's an external declaration
  if (!decl.body) {
    return;
  }

  // Create entry block
  auto *entry = llvm::BasicBlock::Create(ctx_, "entry", func);
  builder_.SetInsertPoint(entry);

  // Save old named values and set up new scope
  auto savedNamedValues = namedValues_;
  namedValues_.clear();

  // Create allocas for all parameters
  idx = 0;
  for (auto &arg : func->args()) {
    auto *alloca =
        createEntryBlockAlloca(func, std::string(arg.getName()), arg.getType());
    builder_.CreateStore(&arg, alloca);
    namedValues_[std::string(arg.getName())] = alloca;
    ++idx;
  }

  // Emit function body
  if (decl.body) {
    for (auto &stmt : decl.body->statements) {
      emitStmt(*stmt);
    }
  }

  // If the block is not terminated, add a return
  if (!builder_.GetInsertBlock()->getTerminator()) {
    if (retType->isVoidTy()) {
      builder_.CreateRetVoid();
    } else {
      builder_.CreateRet(llvm::Constant::getNullValue(retType));
    }
  }

  // Verify the function
  if (llvm::verifyFunction(*func, &llvm::errs())) {
    diag_.emitError(decl.location,
                    "generated invalid IR for function '" + decl.name + "'");
    func->eraseFromParent();
  }

  // Restore parent scope
  namedValues_ = savedNamedValues;
}

void IREmitter::emitStructDecl(ast::StructDecl &decl) {
  std::vector<llvm::Type *> fieldTypes;
  for (auto &field : decl.fields) {
    if (field.type) {
      fieldTypes.push_back(typeMapper_.mapType(*field.type));
    }
  }
  llvm::StructType::create(ctx_, fieldTypes, decl.name);
}

void IREmitter::emitEnumDecl(ast::EnumDecl & /*decl*/) {
  // Simple enums become i32 tags; variants with data become tagged unions.
  // Full implementation in a future pass.
}

// -----------------------------------------------------------------------
// Statement emission
// -----------------------------------------------------------------------

void IREmitter::emitStmt(ast::Stmt &stmt) {
  switch (stmt.kind) {
  case ast::Stmt::Kind::Let:
    emitLetStmt(static_cast<ast::LetStmt &>(stmt));
    break;
  case ast::Stmt::Kind::Return:
    emitReturnStmt(static_cast<ast::ReturnStmt &>(stmt));
    break;
  case ast::Stmt::Kind::If:
    emitIfStmt(static_cast<ast::IfStmt &>(stmt));
    break;
  case ast::Stmt::Kind::For:
    emitForStmt(static_cast<ast::ForStmt &>(stmt));
    break;
  case ast::Stmt::Kind::While:
    emitWhileStmt(static_cast<ast::WhileStmt &>(stmt));
    break;
  case ast::Stmt::Kind::Loop:
    emitLoopStmt(static_cast<ast::LoopStmt &>(stmt));
    break;
  case ast::Stmt::Kind::Block:
    emitBlockStmt(static_cast<ast::BlockStmt &>(stmt));
    break;
  case ast::Stmt::Kind::Expr:
    emitExprStmt(static_cast<ast::ExprStmt &>(stmt));
    break;
  case ast::Stmt::Kind::Break:
    if (!loopStack_.empty()) {
      builder_.CreateBr(loopStack_.back().breakBlock);
    }
    break;
  case ast::Stmt::Kind::Continue:
    if (!loopStack_.empty()) {
      builder_.CreateBr(loopStack_.back().continueBlock);
    }
    break;
  default:
    break;
  }
}

void IREmitter::emitLetStmt(ast::LetStmt &stmt) {
  llvm::Type *varType = nullptr;
  if (stmt.type) {
    varType = typeMapper_.mapType(*stmt.type);
  } else {
    varType = llvm::Type::getInt64Ty(ctx_); // fallback
  }

  auto *func = builder_.GetInsertBlock()->getParent();
  auto *alloca = createEntryBlockAlloca(func, stmt.name, varType);

  if (stmt.initializer) {
    auto *initVal = emitExpr(*stmt.initializer);
    if (initVal) {
      // Insert implicit integer cast if the initializer type differs
      if (varType->isIntegerTy() && initVal->getType()->isIntegerTy() &&
          varType != initVal->getType()) {
        unsigned varBits = varType->getIntegerBitWidth();
        unsigned initBits = initVal->getType()->getIntegerBitWidth();
        if (varBits < initBits) {
          initVal = builder_.CreateTrunc(initVal, varType, "trunc");
        } else {
          initVal = builder_.CreateSExt(initVal, varType, "sext");
        }
      }
      builder_.CreateStore(initVal, alloca);
    }
  }

  namedValues_[stmt.name] = alloca;
}

void IREmitter::emitReturnStmt(ast::ReturnStmt &stmt) {
  if (stmt.value) {
    auto *val = emitExpr(*stmt.value);
    if (val) {
      builder_.CreateRet(val);
    } else {
      builder_.CreateRetVoid();
    }
  } else {
    builder_.CreateRetVoid();
  }
}

void IREmitter::emitIfStmt(ast::IfStmt &stmt) {
  auto *condVal = emitExpr(*stmt.condition);
  if (!condVal)
    return;

  auto *func = builder_.GetInsertBlock()->getParent();

  auto *thenBB = llvm::BasicBlock::Create(ctx_, "then", func);
  auto *elseBB = llvm::BasicBlock::Create(ctx_, "else");
  auto *mergeBB = llvm::BasicBlock::Create(ctx_, "ifcont");

  if (stmt.elseBranch) {
    builder_.CreateCondBr(condVal, thenBB, elseBB);
  } else {
    builder_.CreateCondBr(condVal, thenBB, mergeBB);
  }

  // Then block
  builder_.SetInsertPoint(thenBB);
  emitStmt(*stmt.thenBranch);
  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateBr(mergeBB);
  }

  // Else block
  if (stmt.elseBranch) {
    func->insert(func->end(), elseBB);
    builder_.SetInsertPoint(elseBB);
    emitStmt(*stmt.elseBranch);
    if (!builder_.GetInsertBlock()->getTerminator()) {
      builder_.CreateBr(mergeBB);
    }
  }

  // Merge block
  func->insert(func->end(), mergeBB);
  builder_.SetInsertPoint(mergeBB);
}

void IREmitter::emitWhileStmt(ast::WhileStmt &stmt) {
  auto *func = builder_.GetInsertBlock()->getParent();

  auto *condBB = llvm::BasicBlock::Create(ctx_, "while.cond", func);
  auto *bodyBB = llvm::BasicBlock::Create(ctx_, "while.body", func);
  auto *exitBB = llvm::BasicBlock::Create(ctx_, "while.exit", func);

  builder_.CreateBr(condBB);

  // Condition
  builder_.SetInsertPoint(condBB);
  auto *condVal = emitExpr(*stmt.condition);
  if (condVal) {
    builder_.CreateCondBr(condVal, bodyBB, exitBB);
  }

  // Body
  builder_.SetInsertPoint(bodyBB);
  loopStack_.push_back({exitBB, condBB});
  emitStmt(*stmt.body);
  loopStack_.pop_back();
  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateBr(condBB);
  }

  builder_.SetInsertPoint(exitBB);
}

void IREmitter::emitForStmt(ast::ForStmt &stmt) {
  // For now, emit as a counted loop skeleton.
  // Full iterator protocol requires runtime support.
  auto *func = builder_.GetInsertBlock()->getParent();

  auto *condBB = llvm::BasicBlock::Create(ctx_, "for.cond", func);
  auto *bodyBB = llvm::BasicBlock::Create(ctx_, "for.body", func);
  auto *exitBB = llvm::BasicBlock::Create(ctx_, "for.exit", func);

  builder_.CreateBr(condBB);
  builder_.SetInsertPoint(condBB);
  // Placeholder: always enter body once, then exit
  builder_.CreateBr(bodyBB);

  builder_.SetInsertPoint(bodyBB);
  loopStack_.push_back({exitBB, condBB});
  emitStmt(*stmt.body);
  loopStack_.pop_back();
  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateBr(exitBB);
  }

  builder_.SetInsertPoint(exitBB);
}

void IREmitter::emitLoopStmt(ast::LoopStmt &stmt) {
  auto *func = builder_.GetInsertBlock()->getParent();

  auto *bodyBB = llvm::BasicBlock::Create(ctx_, "loop.body", func);
  auto *exitBB = llvm::BasicBlock::Create(ctx_, "loop.exit", func);

  builder_.CreateBr(bodyBB);
  builder_.SetInsertPoint(bodyBB);

  loopStack_.push_back({exitBB, bodyBB});
  emitStmt(*stmt.body);
  loopStack_.pop_back();

  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateBr(bodyBB);
  }

  builder_.SetInsertPoint(exitBB);
}

void IREmitter::emitBlockStmt(ast::BlockStmt &stmt) {
  for (auto &s : stmt.statements) {
    emitStmt(*s);
  }
}

void IREmitter::emitExprStmt(ast::ExprStmt &stmt) {
  emitExpr(*stmt.expression);
}

// -----------------------------------------------------------------------
// Expression emission
// -----------------------------------------------------------------------

llvm::Value *IREmitter::emitExpr(ast::Expr &expr) {
  switch (expr.kind) {
  case ast::Expr::Kind::IntLiteral:
    return emitIntLiteral(static_cast<ast::IntLiteralExpr &>(expr));
  case ast::Expr::Kind::FloatLiteral:
    return emitFloatLiteral(static_cast<ast::FloatLiteralExpr &>(expr));
  case ast::Expr::Kind::StringLiteral:
    return emitStringLiteral(static_cast<ast::StringLiteralExpr &>(expr));
  case ast::Expr::Kind::BoolLiteral:
    return emitBoolLiteral(static_cast<ast::BoolLiteralExpr &>(expr));
  case ast::Expr::Kind::Ident:
    return emitIdent(static_cast<ast::IdentExpr &>(expr));
  case ast::Expr::Kind::Binary:
    return emitBinaryExpr(static_cast<ast::BinaryExpr &>(expr));
  case ast::Expr::Kind::Unary:
    return emitUnaryExpr(static_cast<ast::UnaryExpr &>(expr));
  case ast::Expr::Kind::Call:
    return emitCallExpr(static_cast<ast::CallExpr &>(expr));
  case ast::Expr::Kind::If:
    return emitIfExpr(static_cast<ast::IfExpr &>(expr));
  case ast::Expr::Kind::Block:
    return emitBlockExpr(static_cast<ast::BlockExpr &>(expr));
  case ast::Expr::Kind::Assign:
    return emitAssignExpr(static_cast<ast::AssignExpr &>(expr));
  default:
    return nullptr;
  }
}

llvm::Value *IREmitter::emitIntLiteral(ast::IntLiteralExpr &expr) {
  return llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx_),
                                static_cast<uint64_t>(expr.value), true);
}

llvm::Value *IREmitter::emitFloatLiteral(ast::FloatLiteralExpr &expr) {
  return llvm::ConstantFP::get(llvm::Type::getDoubleTy(ctx_), expr.value);
}

llvm::Value *IREmitter::emitStringLiteral(ast::StringLiteralExpr &expr) {
  return builder_.CreateGlobalString(expr.value, "str");
}

llvm::Value *IREmitter::emitBoolLiteral(ast::BoolLiteralExpr &expr) {
  return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx_),
                                expr.value ? 1 : 0);
}

llvm::Value *IREmitter::emitIdent(ast::IdentExpr &expr) {
  auto it = namedValues_.find(expr.name);
  if (it == namedValues_.end()) {
    // Could be a function name
    auto *func = module_.getFunction(expr.name);
    if (func)
      return func;

    diag_.emitError(expr.location, "unknown variable '" + expr.name + "'");
    return nullptr;
  }
  return builder_.CreateLoad(it->second->getAllocatedType(), it->second,
                             expr.name);
}

llvm::Value *IREmitter::emitBinaryExpr(ast::BinaryExpr &expr) {
  auto *lhs = emitExpr(*expr.lhs);
  auto *rhs = emitExpr(*expr.rhs);
  if (!lhs || !rhs)
    return nullptr;

  // Coerce integer types to match (e.g., i32 op i64 → widen to i64,
  // or truncate the literal side to match the variable side)
  if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy() &&
      lhs->getType() != rhs->getType()) {
    unsigned lhsBits = lhs->getType()->getIntegerBitWidth();
    unsigned rhsBits = rhs->getType()->getIntegerBitWidth();
    if (lhsBits > rhsBits) {
      rhs = builder_.CreateSExt(rhs, lhs->getType(), "sext");
    } else {
      lhs = builder_.CreateSExt(lhs, rhs->getType(), "sext");
    }
  }

  // Integer operations (default for now)
  bool isFloat = lhs->getType()->isFloatingPointTy();

  switch (expr.op) {
  case ast::BinaryOp::Add:
    return isFloat ? builder_.CreateFAdd(lhs, rhs, "addtmp")
                   : builder_.CreateAdd(lhs, rhs, "addtmp");
  case ast::BinaryOp::Sub:
    return isFloat ? builder_.CreateFSub(lhs, rhs, "subtmp")
                   : builder_.CreateSub(lhs, rhs, "subtmp");
  case ast::BinaryOp::Mul:
    return isFloat ? builder_.CreateFMul(lhs, rhs, "multmp")
                   : builder_.CreateMul(lhs, rhs, "multmp");
  case ast::BinaryOp::Div:
    return isFloat ? builder_.CreateFDiv(lhs, rhs, "divtmp")
                   : builder_.CreateSDiv(lhs, rhs, "divtmp");
  case ast::BinaryOp::Mod:
    return isFloat ? builder_.CreateFRem(lhs, rhs, "modtmp")
                   : builder_.CreateSRem(lhs, rhs, "modtmp");
  case ast::BinaryOp::Equal:
    return isFloat ? builder_.CreateFCmpOEQ(lhs, rhs, "eqtmp")
                   : builder_.CreateICmpEQ(lhs, rhs, "eqtmp");
  case ast::BinaryOp::NotEqual:
    return isFloat ? builder_.CreateFCmpONE(lhs, rhs, "netmp")
                   : builder_.CreateICmpNE(lhs, rhs, "netmp");
  case ast::BinaryOp::Less:
    return isFloat ? builder_.CreateFCmpOLT(lhs, rhs, "lttmp")
                   : builder_.CreateICmpSLT(lhs, rhs, "lttmp");
  case ast::BinaryOp::LessEqual:
    return isFloat ? builder_.CreateFCmpOLE(lhs, rhs, "letmp")
                   : builder_.CreateICmpSLE(lhs, rhs, "letmp");
  case ast::BinaryOp::Greater:
    return isFloat ? builder_.CreateFCmpOGT(lhs, rhs, "gttmp")
                   : builder_.CreateICmpSGT(lhs, rhs, "gttmp");
  case ast::BinaryOp::GreaterEqual:
    return isFloat ? builder_.CreateFCmpOGE(lhs, rhs, "getmp")
                   : builder_.CreateICmpSGE(lhs, rhs, "getmp");
  case ast::BinaryOp::And:
    return builder_.CreateAnd(lhs, rhs, "andtmp");
  case ast::BinaryOp::Or:
    return builder_.CreateOr(lhs, rhs, "ortmp");
  case ast::BinaryOp::BitAnd:
    return builder_.CreateAnd(lhs, rhs, "bandtmp");
  case ast::BinaryOp::BitOr:
    return builder_.CreateOr(lhs, rhs, "bortmp");
  case ast::BinaryOp::BitXor:
    return builder_.CreateXor(lhs, rhs, "bxortmp");
  case ast::BinaryOp::ShiftLeft:
    return builder_.CreateShl(lhs, rhs, "shltmp");
  case ast::BinaryOp::ShiftRight:
    return builder_.CreateAShr(lhs, rhs, "ashrtmp");
  default:
    return nullptr;
  }
}

llvm::Value *IREmitter::emitUnaryExpr(ast::UnaryExpr &expr) {
  auto *operand = emitExpr(*expr.operand);
  if (!operand)
    return nullptr;

  switch (expr.op) {
  case ast::UnaryOp::Negate:
    if (operand->getType()->isFloatingPointTy()) {
      return builder_.CreateFNeg(operand, "negtmp");
    }
    return builder_.CreateNeg(operand, "negtmp");
  case ast::UnaryOp::Not:
    return builder_.CreateNot(operand, "nottmp");
  case ast::UnaryOp::BitwiseNot:
    return builder_.CreateNot(operand, "bnotmp");
  default:
    return nullptr;
  }
}

llvm::Value *IREmitter::emitCallExpr(ast::CallExpr &expr) {
  // For now, handle direct calls via IdentExpr callee
  std::string calleeName;
  if (expr.callee->kind == ast::Expr::Kind::Ident) {
    calleeName = static_cast<ast::IdentExpr &>(*expr.callee).name;
  } else if (expr.callee->kind == ast::Expr::Kind::Path) {
    auto &pe = static_cast<ast::PathExpr &>(*expr.callee);
    for (size_t i = 0; i < pe.segments.size(); ++i) {
      if (i > 0)
        calleeName += "::";
      calleeName += pe.segments[i];
    }
  }

  auto *calleeFunc = module_.getFunction(calleeName);
  if (!calleeFunc) {
    diag_.emitError(expr.location, "unknown function '" + calleeName + "'");
    return nullptr;
  }

  std::vector<llvm::Value *> args;
  for (auto &arg : expr.arguments) {
    auto *val = emitExpr(*arg);
    if (!val)
      return nullptr;
    args.push_back(val);
  }

  if (calleeFunc->getReturnType()->isVoidTy()) {
    builder_.CreateCall(calleeFunc, args);
    return nullptr;
  }
  return builder_.CreateCall(calleeFunc, args, "calltmp");
}

llvm::Value *IREmitter::emitIfExpr(ast::IfExpr &expr) {
  auto *condVal = emitExpr(*expr.condition);
  if (!condVal)
    return nullptr;

  auto *func = builder_.GetInsertBlock()->getParent();

  auto *thenBB = llvm::BasicBlock::Create(ctx_, "then", func);
  auto *elseBB = llvm::BasicBlock::Create(ctx_, "else");
  auto *mergeBB = llvm::BasicBlock::Create(ctx_, "ifcont");

  builder_.CreateCondBr(condVal, thenBB, elseBB);

  // Then
  builder_.SetInsertPoint(thenBB);
  auto *thenVal = emitExpr(*expr.thenExpr);
  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateBr(mergeBB);
  }
  thenBB = builder_.GetInsertBlock();

  // Else
  func->insert(func->end(), elseBB);
  builder_.SetInsertPoint(elseBB);
  llvm::Value *elseVal = nullptr;
  if (expr.elseExpr) {
    elseVal = emitExpr(*expr.elseExpr);
  }
  if (!builder_.GetInsertBlock()->getTerminator()) {
    builder_.CreateBr(mergeBB);
  }
  elseBB = builder_.GetInsertBlock();

  // Merge
  func->insert(func->end(), mergeBB);
  builder_.SetInsertPoint(mergeBB);

  if (thenVal && elseVal && thenVal->getType() == elseVal->getType()) {
    auto *phi = builder_.CreatePHI(thenVal->getType(), 2, "iftmp");
    phi->addIncoming(thenVal, thenBB);
    phi->addIncoming(elseVal, elseBB);
    return phi;
  }

  return thenVal;
}

llvm::Value *IREmitter::emitBlockExpr(ast::BlockExpr &expr) {
  for (auto &s : expr.statements) {
    emitStmt(*s);
  }
  if (expr.finalExpr) {
    return emitExpr(*expr.finalExpr);
  }
  return nullptr;
}

llvm::Value *IREmitter::emitAssignExpr(ast::AssignExpr &expr) {
  auto *val = emitExpr(*expr.value);
  if (!val)
    return nullptr;

  if (expr.target->kind == ast::Expr::Kind::Ident) {
    auto &ident = static_cast<ast::IdentExpr &>(*expr.target);
    auto it = namedValues_.find(ident.name);
    if (it != namedValues_.end()) {
      builder_.CreateStore(val, it->second);
      return val;
    }
  }
  return nullptr;
}

// -----------------------------------------------------------------------
// Helper
// -----------------------------------------------------------------------

llvm::AllocaInst *IREmitter::createEntryBlockAlloca(llvm::Function *func,
                                                    const std::string &name,
                                                    llvm::Type *type) {
  llvm::IRBuilder<> tempBuilder(&func->getEntryBlock(),
                                func->getEntryBlock().begin());
  return tempBuilder.CreateAlloca(type, nullptr, name);
}

} // namespace flux
