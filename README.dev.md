# Flux — Developer Guide

Instructions for building, testing, and developing the Flux compiler.

## Prerequisites

| Tool               | Version                         | Notes                                                             |
| ------------------ | ------------------------------- | ----------------------------------------------------------------- |
| **CMake**          | ≥ 3.20                          | Build system generator                                            |
| **Ninja**          | any                             | Build backend                                                     |
| **LLVM**           | 18.x                            | Prebuilt development libraries                                    |
| **C++20 compiler** | MSVC 2022 / GCC 13+ / Clang 17+ | Must support C++20 and build without RTTI (`/GR-` or `-fno-rtti`) |

### Windows-Specific

- **Visual Studio 2022** (Community or higher) with the "Desktop development with C++" workload
- Or standalone MSVC Build Tools 2022
- The DIA SDK is auto-detected from your VS installation

## Dockerized Testing (Linux)

For a clean, reproducible Linux environment, use the provided Docker setup:

```bash
# Build and run all internal tests
./scripts/docker-test.sh
```

Or manually:

```bash
docker build -t flux-compiler .
docker run --rm flux-compiler --version
```

## Getting LLVM

Download the **LLVM 18** prebuilt development package and extract it to `llvm-dev/` at the project root. LLVM 18 is the standard target for professional Flux distribution.

```
flux/
└── llvm-dev/
    ├── bin/
    ├── include/
    ├── lib/
    │   └── cmake/
    │       └── llvm/
    │           └── LLVMConfig.cmake   ← CMake finds this
    ├── libexec/
    └── share/
```

The CMake presets reference `${sourceDir}/llvm-dev/lib/cmake/llvm` by default. If your LLVM is elsewhere, pass `-DLLVM_DIR=<path>` manually.

## Building

### Option A: CMake Presets (recommended)

```bash
# Debug build (default)
cmake --preset default
cmake --build build

# Release build
cmake --preset release
cmake --build build

# Release with debug info
cmake --preset relwithdebinfo
cmake --build build
```

### Option B: Makefile Wrapper

```bash
make                # Debug build
make release        # Release build
make clean          # Remove build/
make reconfigure    # Clean + reconfigure
```

### Option C: Manual CMake

```bash
cmake -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_DIR=D:/flux/llvm-dev/lib/cmake/llvm

cmake --build build
```

### Windows with MSVC (Developer Command Prompt)

If using MSVC instead of Clang, open a **Developer Command Prompt for VS 2022** (or run `VsDevCmd.bat -arch=amd64`) and then:

```cmd
cmake --preset default -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl
cmake --build build
```

## Build Outputs

| Artifact         | Path                              |
| ---------------- | --------------------------------- |
| Compiler binary  | `build/tools/flux/flux.exe`       |
| Unit test binary | `build/tests/flux_unit_tests.exe` |
| Runtime library  | `build/runtime/`                  |
| Compile commands | `build/compile_commands.json`     |

## Running the Compiler

```bash
# Show version
./build/tools/flux/flux --version

# Dump token stream
./build/tools/flux/flux --dump-tokens examples/hello.fl

# Dump AST
./build/tools/flux/flux --dump-ast examples/hello.fl

# Compile to LLVM IR
./build/tools/flux/flux examples/hello.fl --emit llvm-ir -o hello.ll

# Full help
./build/tools/flux/flux --help
```

Or via the Makefile:

```bash
make run ARGS="examples/hello.fl --dump-tokens"
make dump-tokens ARGS="examples/hello.fl"
make dump-ast ARGS="examples/hello.fl"
```

## Testing

### Run All Tests

```bash
# Via CMake/CTest
ctest --test-dir build --output-on-failure

# Via Makefile
make test
```

### Run Specific Tests

```bash
# Run a single test by name
cd build && ctest -R LexerTest.Keywords --output-on-failure

# Run all lexer tests
cd build && ctest -R LexerTest --output-on-failure

# Run all parser tests
cd build && ctest -R ParserTest --output-on-failure

# Run all sema tests
cd build && ctest -R SemaTest --output-on-failure
```

### Run Unit Tests Directly

```bash
# All tests
./build/tests/flux_unit_tests

# GoogleTest filter
./build/tests/flux_unit_tests --gtest_filter="LexerTest.*"
./build/tests/flux_unit_tests --gtest_filter="ParserTest.StructDeclaration"
```

### Test Structure

| Directory                   | Framework  | Coverage                                                       |
| --------------------------- | ---------- | -------------------------------------------------------------- |
| `tests/unit/LexerTest.cpp`  | GoogleTest | Tokenizer — literals, keywords, operators, comments, locations |
| `tests/unit/ParserTest.cpp` | GoogleTest | Parser — declarations, expressions, statements, error recovery |
| `tests/unit/SemaTest.cpp`   | GoogleTest | Semantic analysis — name resolution, duplicates, scoping       |
| `tests/lit/`                | LLVM lit   | End-to-end compiler driver tests                               |

## CMake Options

| Variable            | Default | Description                              |
| ------------------- | ------- | ---------------------------------------- |
| `FLUX_ENABLE_TESTS` | `ON`    | Build unit tests                         |
| `FLUX_ENABLE_LIT`   | `ON`    | Enable LLVM lit tests                    |
| `FLUX_ENABLE_ASAN`  | `OFF`   | AddressSanitizer (Debug, GCC/Clang only) |
| `LLVM_DIR`          | —       | Path to `LLVMConfig.cmake` directory     |
| `CMAKE_BUILD_TYPE`  | —       | `Debug`, `Release`, `RelWithDebInfo`     |

## Project Architecture

```
Source → Lexer → Parser → AST → Sema → CodeGen → LLVM IR → Native Code
```

| Component   | Library       | Description                                |
| ----------- | ------------- | ------------------------------------------ |
| **Lexer**   | `FluxLexer`   | Tokenizes `.fl` source into a token stream |
| **Parser**  | `FluxParser`  | Recursive-descent parser producing an AST  |
| **AST**     | `FluxAST`     | Abstract syntax tree nodes and visitors    |
| **Sema**    | `FluxSema`    | Name resolution and type checking          |
| **CodeGen** | `FluxCodeGen` | LLVM IR emission from the AST              |
| **Common**  | `FluxCommon`  | Source locations, diagnostics engine       |
| **Runtime** | `FluxRuntime` | Minimal C runtime (panic, alloc, I/O)      |
| **Driver**  | `flux` (exe)  | CLI entry point, option parsing, pipeline  |

Each library is built as a static library. The driver links them all together with LLVM.

## VS Code Development

### Setup

The `.vscode/` directory is pre-configured. After cloning:

1. Open the `flux/` folder in VS Code
2. Install recommended extensions (C/C++, CMake Tools)
3. CMake Tools will auto-configure using the `default` preset
4. `Ctrl+Shift+B` to build, `F5` to debug

### Tasks (`Ctrl+Shift+P` → "Run Task")

| Task                       | Description                             |
| -------------------------- | --------------------------------------- |
| Build Flux                 | Build the compiler (default build task) |
| Run Tests                  | Build + run all unit tests              |
| Clean                      | Clean build artifacts                   |
| Reconfigure (Clean)        | Fresh CMake reconfigure                 |
| Run Flux (current file)    | Compile the open `.fl` file             |
| Dump Tokens (current file) | Show token stream                       |
| Dump AST (current file)    | Show AST                                |

### Debug Configurations (`F5`)

| Configuration             | Description                                        |
| ------------------------- | -------------------------------------------------- |
| Debug Flux (current file) | Debug compiler with currently open `.fl` file      |
| Debug Flux (hello.fl)     | Debug compiler with `examples/hello.fl --dump-ast` |
| Debug Flux (dump tokens)  | Debug compiler with `--dump-tokens`                |
| Debug Unit Tests          | Debug the GoogleTest binary                        |

## Adding a New Compiler Pass

1. Create header in `include/flux/YourPass/`
2. Create source in `lib/YourPass/`
3. Add a `lib/YourPass/CMakeLists.txt`:
   ```cmake
   add_library(FluxYourPass STATIC YourPass.cpp)
   flux_set_target_options(FluxYourPass)
   target_link_libraries(FluxYourPass PRIVATE FluxAST FluxCommon)
   ```
4. Add `add_subdirectory(lib/YourPass)` to the root `CMakeLists.txt`
5. Link it in `tools/flux/CMakeLists.txt`
6. Add tests in `tests/unit/YourPassTest.cpp`

## Adding Tests

### Unit Test

Add a new `TEST()` in the appropriate `tests/unit/*Test.cpp` file:

```cpp
TEST(LexerTest, YourNewTest) {
    std::string source = "let x: Int32 = 42;";
    flux::Lexer lexer(source, "test.fl");
    auto tokens = lexer.tokenize();
    // assertions...
}
```

Tests are automatically discovered by GoogleTest — just rebuild and run.

### Lit Test

Create a `.fl` file in `tests/lit/`:

```
// RUN: %flux --dump-tokens %s | FileCheck %s
// CHECK: KwLet 'let'
let x: Int32 = 42;
```

## Documentation Site

```bash
cd docs
npm install
npm run dev       # Dev server at http://localhost:5173
npm run build     # Build static site to .vitepress/dist/
npm run preview   # Preview the built site
```

The documentation uses VitePress with a custom Shiki grammar for Flux syntax highlighting — all ` ```flux ` code blocks are highlighted automatically.

## Professional Distribution

### Releasing

Flux uses **Semantic Versioning** and **Conventional Commits**.

1. **Commit Changes**: Use `npm run commit` for a guided conventional commit wizard.
2. **Tagging**: When ready for a release, create a git tag:
   ```bash
   git tag -a v0.1.0 -m "Release v0.1.0"
   git push origin v0.1.0
   ```
3. **Automated Release**: Pushing a tag triggers the [GitHub Actions Release Workflow](.github/workflows/release.yml), which:
   - Builds binaries for Windows, Linux, and macOS.
   - Packages them into installers (ZIP, DEB, DMG, MSI).
   - Builds and packages the VS Code extension (`.vsix`).
   - Creates a GitHub Release and uploads all artifacts.

### Local Packaging

You can test the packaging process locally using CPack:

```bash
# After building in Release mode
cd build
cpack -C Release
```

This will generate the installers in the `build/` directory.

### Pre-release Checklist

Before tagging a new version for distribution, ensure the following files are updated:

1.  **`CMakeLists.txt`**: Update the `project(... VERSION x.y.z)` call (Line 2).
2.  **`package.json`**: Update the `"version": "x.y.z"` field (Line 3).
3.  **`editors/vscode/package.json`**: Update the `"version": "x.y.z"` field (Line 3).
4.  **`CHANGELOG.md`**: Ensure all notable changes are documented.

**Recommended Tooling**:

- Use `npm version <patch|minor|major>` to automatically update `package.json`.
- Use `npm run commit` for conventional commits, which compatible with `semantic-release` for automated changelog generation and tagging.

### Local Environment Recommendation

It is highly recommended to align your local environment with the CI production build:

- **LLVM Version**: Use **LLVM 18.x**. It is the stable target for all distribution packages.
- **Tools**: Ensure `Ninja` and a C++20 compatible compiler are in your PATH.

```
gh release delete v0.1.0 --repo otabekoff/flux --yes && git tag -d v0.1.0 && git push origin :refs/tags/v0.1.0 && git tag v0.1.0 && git push origin v0.1.0
```

```
cmake -B build -G Ninja
cmake --build build
```
