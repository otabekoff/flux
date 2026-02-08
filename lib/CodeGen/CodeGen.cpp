#include "flux/CodeGen/CodeGen.h"

#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

namespace flux {

CodeGen::CodeGen(DiagnosticEngine& diag, const CodeGenOptions& opts)
    : diag_(diag), opts_(opts) {
    // Initialise only the LLVM targets we linked against
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmParser();
    LLVMInitializeX86AsmPrinter();

    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64Target();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64AsmParser();
    LLVMInitializeAArch64AsmPrinter();

    LLVMInitializeWebAssemblyTargetInfo();
    LLVMInitializeWebAssemblyTarget();
    LLVMInitializeWebAssemblyTargetMC();
    LLVMInitializeWebAssemblyAsmParser();
    LLVMInitializeWebAssemblyAsmPrinter();
}

bool CodeGen::initializeTarget() {
    std::string tripleStr = opts_.targetTriple;
    if (tripleStr.empty()) {
        tripleStr = llvm::sys::getDefaultTargetTriple();
    }
    llvm::Triple triple(tripleStr);
    llvmModule_->setTargetTriple(triple.getTriple());

    std::string error;
    auto* target = llvm::TargetRegistry::lookupTarget(tripleStr, error);
    if (!target) {
        diag_.emitError({}, "failed to lookup target '" + tripleStr + "': " + error);
        return false;
    }

    auto relocModel = llvm::Reloc::PIC_;
    llvm::TargetOptions targetOpts;

    llvm::CodeGenOptLevel cgOptLevel;
    switch (opts_.optimizationLevel) {
    case 0:  cgOptLevel = llvm::CodeGenOptLevel::None;       break;
    case 1:  cgOptLevel = llvm::CodeGenOptLevel::Less;       break;
    case 2:  cgOptLevel = llvm::CodeGenOptLevel::Default;    break;
    default: cgOptLevel = llvm::CodeGenOptLevel::Aggressive; break;
    }

    targetMachine_ = target->createTargetMachine(
        triple.getTriple(), opts_.cpu, opts_.features, targetOpts, relocModel,
        std::nullopt, cgOptLevel);

    if (!targetMachine_) {
        diag_.emitError({}, "failed to create target machine");
        return false;
    }

    llvmModule_->setDataLayout(targetMachine_->createDataLayout());
    return true;
}

bool CodeGen::generate(ast::Module& module) {
    llvmModule_ = std::make_unique<llvm::Module>(module.name, context_);

    if (!initializeTarget()) {
        return false;
    }

    // Emit IR for all declarations
    IREmitter emitter(context_, *llvmModule_, diag_);
    for (auto& decl : module.declarations) {
        emitter.emitDecl(*decl);
    }

    // Verify the module
    std::string verifyErrors;
    llvm::raw_string_ostream verifyStream(verifyErrors);
    if (llvm::verifyModule(*llvmModule_, &verifyStream)) {
        diag_.emitError({}, "module verification failed:\n" + verifyErrors);
        return false;
    }

    // Run optimization passes
    if (opts_.optimizationLevel > 0) {
        if (!runOptimizationPasses()) {
            return false;
        }
    }

    return true;
}

bool CodeGen::runOptimizationPasses() {
    llvm::PassBuilder passBuilder(targetMachine_);

    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;

    passBuilder.registerModuleAnalyses(mam);
    passBuilder.registerCGSCCAnalyses(cgam);
    passBuilder.registerFunctionAnalyses(fam);
    passBuilder.registerLoopAnalyses(lam);
    passBuilder.crossRegisterProxies(lam, fam, cgam, mam);

    llvm::OptimizationLevel optLevel;
    switch (opts_.optimizationLevel) {
    case 1:  optLevel = llvm::OptimizationLevel::O1; break;
    case 2:  optLevel = llvm::OptimizationLevel::O2; break;
    default: optLevel = llvm::OptimizationLevel::O3; break;
    }

    auto mpm = passBuilder.buildPerModuleDefaultPipeline(optLevel);
    mpm.run(*llvmModule_, mam);
    return true;
}

bool CodeGen::writeOutput(const std::string& filename) {
    if (opts_.outputFormat == OutputFormat::LLVMIR) {
        std::error_code ec;
        llvm::raw_fd_ostream out(filename, ec, llvm::sys::fs::OF_Text);
        if (ec) {
            diag_.emitError({}, "could not open file '" + filename + "': " +
                            ec.message());
            return false;
        }
        llvmModule_->print(out, nullptr);
        return true;
    }

    if (opts_.outputFormat == OutputFormat::Bitcode) {
        std::error_code ec;
        llvm::raw_fd_ostream out(filename, ec, llvm::sys::fs::OF_None);
        if (ec) {
            diag_.emitError({}, "could not open file '" + filename + "': " +
                            ec.message());
            return false;
        }
        llvm::WriteBitcodeToFile(*llvmModule_, out);
        return true;
    }

    return emitToFile(filename);
}

bool CodeGen::emitToFile(const std::string& filename) {
    std::error_code ec;
    llvm::raw_fd_ostream out(filename, ec, llvm::sys::fs::OF_None);
    if (ec) {
        diag_.emitError({}, "could not open file '" + filename + "': " +
                        ec.message());
        return false;
    }

    llvm::legacy::PassManager pm;

    auto fileType = (opts_.outputFormat == OutputFormat::Assembly)
        ? llvm::CodeGenFileType::AssemblyFile
        : llvm::CodeGenFileType::ObjectFile;

    if (targetMachine_->addPassesToEmitFile(pm, out, nullptr, fileType)) {
        diag_.emitError({}, "target machine cannot emit this file type");
        return false;
    }

    pm.run(*llvmModule_);
    out.flush();
    return true;
}

} // namespace flux
