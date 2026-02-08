#pragma once

#include "flux/AST/AST.h"
#include "flux/CodeGen/IREmitter.h"
#include "flux/Common/Diagnostics.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Target/TargetMachine.h>

#include <memory>
#include <string>

namespace flux {

/// Output format for code generation.
enum class OutputFormat {
    LLVMIR,      // .ll — textual LLVM IR
    Bitcode,     // .bc — LLVM bitcode
    Assembly,    // .s  — native assembly
    Object,      // .o  — native object file
    Executable,  // linked executable
};

/// Options controlling code generation.
struct CodeGenOptions {
    std::string targetTriple;   // e.g. "x86_64-pc-windows-msvc"
    std::string cpu = "generic";
    std::string features;
    OutputFormat outputFormat = OutputFormat::Object;
    int optimizationLevel = 0;  // 0-3
    bool debugInfo = false;
};

/// Top-level code generation driver.
///
/// Takes a type-checked AST module and produces LLVM IR, bitcode,
/// assembly, or object code.
class CodeGen {
public:
    CodeGen(DiagnosticEngine& diag, const CodeGenOptions& opts);

    /// Generate code for the given module.
    /// Returns true on success.
    bool generate(ast::Module& module);

    /// Write the generated output to a file.
    bool writeOutput(const std::string& filename);

    /// Get the LLVM module (for inspection/testing).
    llvm::Module* getLLVMModule() { return llvmModule_.get(); }

private:
    bool initializeTarget();
    bool runOptimizationPasses();
    bool emitToFile(const std::string& filename);

    DiagnosticEngine& diag_;
    CodeGenOptions opts_;

    llvm::LLVMContext context_;
    std::unique_ptr<llvm::Module> llvmModule_;
    llvm::TargetMachine* targetMachine_ = nullptr;
};

} // namespace flux
