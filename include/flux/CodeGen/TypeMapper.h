#pragma once

#include "flux/AST/AST.h"
#include "flux/AST/Decl.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Stmt.h"
#include "flux/AST/Type.h"
#include "flux/Common/Diagnostics.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace flux {

/// Maps Flux type nodes to LLVM types.
class TypeMapper {
public:
    explicit TypeMapper(llvm::LLVMContext& ctx);

    /// Map a Flux AST type to an LLVM type.
    llvm::Type* mapType(const ast::TypeNode& type);

    /// Get the LLVM type for a built-in type by name.
    llvm::Type* getBuiltinType(const std::string& name);

    /// Get the void type.
    llvm::Type* getVoidType();

private:
    llvm::LLVMContext& ctx_;
    std::unordered_map<std::string, llvm::Type*> builtinTypes_;
    void initBuiltinTypes();
};

} // namespace flux
