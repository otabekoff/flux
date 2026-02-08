# Flux Strategic Roadmap: The Self-Hosting Milestone

This document outlines the architectural and engineering requirements for Flux to reach the "Self-Hosting" milestone (compiling the Flux compiler using Flux itself).

## 1. Goal Description

Self-hosting is the ultimate maturity test for a new programming language. It proves the language is expressive and robust enough to handle its own complex logic (parsing, semantic analysis, and code generation).

## 2. Technical Prerequisites

### Phase A: Language Expressiveness

To compile the current C++ codebase, Flux needs equivalent features:

- **Advanced Pattern Matching**: For AST traversal.
- **Generics / Templates**: For collection types (Vectors, Maps).
- **Comprehensive Error Handling**: Result/Option types or equivalent.
- **Stable FFI**: Ability to link with LLVM's C/C++ API.

### Phase B: Standard Library (FluxStd)

- **File I/O**: Reading source files.
- **Memory Management**: Arena allocators (critical for compiler performance).
- **String Manipulation**: Robust UTF-8 and rope structures.

## 3. Implementation Path

| Milestone                 | Description                                                 | Priority |
| ------------------------- | ----------------------------------------------------------- | -------- |
| **M1: Bootstrap Lexer**   | Port the Lexer to Flux. Verify it can tokenize Flux source. | High     |
| **M2: Bootstrap Parser**  | Port the Recursive Descent Parser.                          | High     |
| **M3: LLVM Bindings**     | Create Flux-friendly bindings for LLVM-C API.               | Medium   |
| **M4: The "Full Bridge"** | Compile the entire pipeline in Flux.                        | Low      |

## 4. Stability Guarantees

- **Regression Suite**: Every self-hosted build must pass the existing `ctest` and `lit` suites.
- **Fuzzing Consistency**: The self-hosted lexer/parser must pass the same LibFuzzer stress tests as the C++ version.

## 5. Timeline

Self-hosting is a long-term goal. The current focus remains on **Stability, Tooling, and Developer Experience** (Phase 1-3).
