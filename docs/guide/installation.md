# Installation

## Prerequisites

- **CMake** ≥ 3.20
- **Ninja** build system
- **LLVM 21** development libraries
- **C++20 compiler** (MSVC 2022, GCC 13+, or Clang 17+)

## Building from Source

### 1. Clone the repository

```bash
git clone https://github.com/flux-lang/flux.git
cd flux
```

### 2. Set up LLVM

Download the LLVM 21 prebuilt development package and extract it to `llvm-dev/` in the project root:

```bash
# The CMake presets expect LLVM at ./llvm-dev/
# LLVMConfig.cmake should be at ./llvm-dev/lib/cmake/llvm/
```

### 3. Configure and build

```bash
# Using CMake presets (recommended)
cmake --preset default
cmake --build build

# Or using the Makefile wrapper
make
```

### 4. Run tests

```bash
make test
# or
ctest --test-dir build --output-on-failure
```

### 5. Verify installation

```bash
./build/tools/flux/flux --version
./build/tools/flux/flux examples/hello.fl --dump-tokens
```

## Editor Support

Install the Flux VS Code extension from `editors/vscode/` for syntax highlighting, snippets, and bracket matching.

```bash
cd editors/vscode
npx @vscode/vsce package
code --install-extension flux-lang-0.1.0.vsix
```
