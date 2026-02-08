#include "flux/CodeGen/TypeMapper.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

namespace flux {

TypeMapper::TypeMapper(llvm::LLVMContext& ctx) : ctx_(ctx) {
    initBuiltinTypes();
}

void TypeMapper::initBuiltinTypes() {
    // Integer types
    builtinTypes_["Int8"]   = llvm::Type::getInt8Ty(ctx_);
    builtinTypes_["Int16"]  = llvm::Type::getInt16Ty(ctx_);
    builtinTypes_["Int32"]  = llvm::Type::getInt32Ty(ctx_);
    builtinTypes_["Int64"]  = llvm::Type::getInt64Ty(ctx_);
    builtinTypes_["UInt8"]  = llvm::Type::getInt8Ty(ctx_);
    builtinTypes_["UInt16"] = llvm::Type::getInt16Ty(ctx_);
    builtinTypes_["UInt32"] = llvm::Type::getInt32Ty(ctx_);
    builtinTypes_["UInt64"] = llvm::Type::getInt64Ty(ctx_);

    // Floating-point types
    builtinTypes_["Float32"] = llvm::Type::getFloatTy(ctx_);
    builtinTypes_["Float64"] = llvm::Type::getDoubleTy(ctx_);

    // Bool is i1 in LLVM
    builtinTypes_["Bool"] = llvm::Type::getInt1Ty(ctx_);

    // Char is i32 (Unicode scalar value)
    builtinTypes_["Char"] = llvm::Type::getInt32Ty(ctx_);

    // String is a pointer to i8 (will be a struct in full implementation)
    builtinTypes_["String"] = llvm::PointerType::getUnqual(
        llvm::Type::getInt8Ty(ctx_));

    // Void
    builtinTypes_["Void"] = llvm::Type::getVoidTy(ctx_);
}

llvm::Type* TypeMapper::getVoidType() {
    return builtinTypes_["Void"];
}

llvm::Type* TypeMapper::getBuiltinType(const std::string& name) {
    auto it = builtinTypes_.find(name);
    if (it != builtinTypes_.end()) {
        return it->second;
    }
    return nullptr;
}

llvm::Type* TypeMapper::mapType(const ast::TypeNode& type) {
    switch (type.kind) {
    case ast::TypeNode::Kind::Named: {
        auto& nt = static_cast<const ast::NamedType&>(type);
        if (nt.path.size() == 1) {
            auto* builtin = getBuiltinType(nt.path[0]);
            if (builtin) return builtin;
        }
        // For user-defined types, return an opaque pointer for now
        return llvm::PointerType::getUnqual(ctx_);
    }

    case ast::TypeNode::Kind::Generic: {
        auto& gt = static_cast<const ast::GenericType&>(type);
        // Monomorphisation would happen here
        return mapType(*gt.base);
    }

    case ast::TypeNode::Kind::Reference:
    case ast::TypeNode::Kind::MutRef:
        // References are pointers in LLVM
        return llvm::PointerType::getUnqual(ctx_);

    case ast::TypeNode::Kind::Tuple: {
        auto& tt = static_cast<const ast::TupleType&>(type);
        std::vector<llvm::Type*> elems;
        for (auto& e : tt.elements) {
            elems.push_back(mapType(*e));
        }
        return llvm::StructType::get(ctx_, elems);
    }

    case ast::TypeNode::Kind::Function: {
        auto& ft = static_cast<const ast::FunctionType&>(type);
        llvm::Type* retType = mapType(*ft.returnType);
        std::vector<llvm::Type*> paramTypes;
        for (auto& p : ft.paramTypes) {
            paramTypes.push_back(mapType(*p));
        }
        auto* fnType = llvm::FunctionType::get(retType, paramTypes, false);
        return llvm::PointerType::getUnqual(fnType);
    }

    case ast::TypeNode::Kind::Array: {
        auto& at = static_cast<const ast::ArrayType&>(type);
        llvm::Type* elemType = mapType(*at.elementType);
        if (at.size.has_value()) {
            return llvm::ArrayType::get(elemType, at.size.value());
        }
        // Dynamic array: pointer (will be a slice struct later)
        return llvm::PointerType::getUnqual(elemType);
    }

    case ast::TypeNode::Kind::Option:
    case ast::TypeNode::Kind::Result:
        // These will be represented as tagged unions
        return llvm::PointerType::getUnqual(ctx_);

    default:
        return llvm::PointerType::getUnqual(ctx_);
    }
}

} // namespace flux
