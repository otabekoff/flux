/// Flux Compiler — CLI Driver
///
/// Usage:
///   flux <input.fl> [options]
///
/// Options:
///   -o <file>         Output file
///   --emit <format>   Output format: llvm-ir, bitcode, asm, obj, exe (default:
///   exe) -O<level>         Optimization level: 0, 1, 2, 3 (default: 0)
///   --target <triple> Target triple (default: host)
///   --dump-ast        Dump the AST to stdout
///   --dump-tokens     Dump the token stream to stdout
///   --help            Show this help message
///   --version         Show version

#include "flux/AST/AST.h"
#include "flux/CodeGen/CodeGen.h"
#include "flux/Common/Diagnostics.h"

#include "flux/Common/SourceLocation.h"
#include "flux/Lexer/Lexer.h"
#include "flux/Parser/Parser.h"
#include "flux/Sema/Sema.h"
#include <memory>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct DriverOptions {
  std::string inputFile;
  std::string outputFile;
  std::string targetTriple;
  flux::OutputFormat outputFormat = flux::OutputFormat::Executable;
  int optimizationLevel = 0;
  bool dumpAST = false;
  bool dumpTokens = false;
  bool showHelp = false;
  bool showVersion = false;
};

void printUsage() {
  std::cout << R"(Flux Compiler v0.1.0

Usage: flux <input.fl> [options]

Options:
  -o <file>         Output file path
  --emit <format>   Output format: llvm-ir, bitcode, asm, obj, exe (default: exe)
  -O0, -O1, -O2, -O3  Optimization level (default: -O0)
  --target <triple> Target triple (default: host)
  --dump-ast        Print the AST to stdout
  --dump-tokens     Print the token stream to stdout
  --help            Show this help message
  --version         Show version information
)";
}

void printVersion() {
  std::cout << "Flux Compiler v0.1.0\n"
            << "Built with LLVM\n";
}

DriverOptions parseArgs(int argc, char *argv[]) {
  DriverOptions opts;

  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--help" || arg == "-h") {
      opts.showHelp = true;
    } else if (arg == "--version" || arg == "-v") {
      opts.showVersion = true;
    } else if (arg == "-o" && i + 1 < argc) {
      opts.outputFile = argv[++i];
    } else if (arg == "--emit" && i + 1 < argc) {
      std::string fmt = argv[++i];
      if (fmt == "llvm-ir")
        opts.outputFormat = flux::OutputFormat::LLVMIR;
      else if (fmt == "bitcode")
        opts.outputFormat = flux::OutputFormat::Bitcode;
      else if (fmt == "asm")
        opts.outputFormat = flux::OutputFormat::Assembly;
      else if (fmt == "obj")
        opts.outputFormat = flux::OutputFormat::Object;
      else if (fmt == "exe")
        opts.outputFormat = flux::OutputFormat::Executable;
      else {
        std::cerr << "error: unknown output format '" << fmt << "'\n";
        std::exit(1);
      }
    } else if (arg == "--target" && i + 1 < argc) {
      opts.targetTriple = argv[++i];
    } else if (arg == "-O0") {
      opts.optimizationLevel = 0;
    } else if (arg == "-O1") {
      opts.optimizationLevel = 1;
    } else if (arg == "-O2") {
      opts.optimizationLevel = 2;
    } else if (arg == "-O3") {
      opts.optimizationLevel = 3;
    } else if (arg == "--dump-ast") {
      opts.dumpAST = true;
    } else if (arg == "--dump-tokens") {
      opts.dumpTokens = true;
    } else if (arg[0] != '-') {
      opts.inputFile = arg;
    } else {
      std::cerr << "error: unknown option '" << arg << "'\n";
      std::exit(1);
    }
  }

  return opts;
}

std::string readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "error: could not open file '" << path << "'\n";
    std::exit(1);
  }
  std::ostringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

std::string deriveOutputFilename(const std::string &input,
                                 flux::OutputFormat format) {
  namespace fs = std::filesystem;
  auto stem = fs::path(input).stem().string();

  switch (format) {
  case flux::OutputFormat::LLVMIR:
    return stem + ".ll";
  case flux::OutputFormat::Bitcode:
    return stem + ".bc";
  case flux::OutputFormat::Assembly:
    return stem + ".s";
  case flux::OutputFormat::Object:
    return stem + ".o";
  case flux::OutputFormat::Executable:
#ifdef _WIN32
    return stem + ".exe";
#else
    return stem;
#endif
  }
  return stem;
}

} // anonymous namespace

int main(int argc, char *argv[]) {
  auto opts = parseArgs(argc, argv);

  if (opts.showHelp) {
    printUsage();
    return 0;
  }
  if (opts.showVersion) {
    printVersion();
    return 0;
  }
  if (opts.inputFile.empty()) {
    std::cerr << "error: no input file\n";
    printUsage();
    return 1;
  }

  // Read input
  std::string source = readFile(opts.inputFile);

  // Set up diagnostics
  flux::DiagnosticEngine diag;

  // Source manager
  flux::SourceManager srcMgr;
  srcMgr.loadFromString(opts.inputFile, source);

  // === Phase 1: Lexical analysis ===
  auto lexer = std::make_unique<flux::Lexer>(source, opts.inputFile, diag);

  if (opts.dumpTokens) {
    auto tokens = lexer->lexAll();
    for (auto &tok : tokens) {
      std::cout << flux::Token::kindToString(tok.kind) << " '" << tok.text
                << "'"
                << " @ " << tok.location.line << ":" << tok.location.column
                << "\n";
    }
    if (diag.getErrorCount() > 0)
      return 1;
    // Re-create lexer for further processing
    lexer = std::make_unique<flux::Lexer>(source, opts.inputFile, diag);
  }

  // === Phase 2: Parsing ===
  flux::Parser parser(*lexer, diag);
  auto module = parser.parseModule();

  if (diag.getErrorCount() > 0) {
    std::cerr << diag.getErrorCount() << " error(s) generated.\n";
    return 1;
  }

  if (opts.dumpAST) {
    // TODO: pretty-print the AST
    std::cout << "Module: " << module->name << "\n";
    std::cout << "  Declarations: " << module->declarations.size() << "\n";
    if (diag.getErrorCount() > 0)
      return 1;
  }

  // === Phase 3: Semantic analysis ===
  flux::Sema sema(diag);
  if (!sema.analyze(*module)) {
    std::cerr << diag.getErrorCount() << " error(s) generated.\n";
    return 1;
  }

  // === Phase 4: Code generation ===
  flux::CodeGenOptions cgOpts;
  cgOpts.targetTriple = opts.targetTriple;
  cgOpts.optimizationLevel = opts.optimizationLevel;
  cgOpts.outputFormat = opts.outputFormat;

  flux::CodeGen codegen(diag, cgOpts);
  if (!codegen.generate(*module)) {
    std::cerr << diag.getErrorCount() << " error(s) generated.\n";
    return 1;
  }

  // Determine output filename
  std::string outFile =
      opts.outputFile.empty()
          ? deriveOutputFilename(opts.inputFile, opts.outputFormat)
          : opts.outputFile;

  if (!codegen.writeOutput(outFile)) {
    std::cerr << "error: failed to write output to '" << outFile << "'\n";
    return 1;
  }

  if (opts.outputFormat == flux::OutputFormat::Executable) {
    // Link the object file into an executable
    // For now, just produce the object file
    std::cout << "Output written to " << outFile << "\n";
  }

  return 0;
}
