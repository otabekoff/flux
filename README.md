# Flux

**Flux** is a modern, production-ready, statically typed programming language designed for systems programming. Every type is stated, every allocation is visible, and there are no hidden conversions or copies.

```flux
module hello;

import std::io;

func main() -> Void {
    let message: String = "Hello, Flux!";
    io::println(message);

    let result: Int32 = add(10, 20);
    io::print_int(result);
}

func add(a: Int32, b: Int32) -> Int32 {
    return a + b;
}
```

## Features

- **Always explicit** — No implicit type conversions, no hidden copies, no magic
- **Ownership & borrowing** — Memory safety without garbage collection (`move`, `&`, `&mut`)
- **Traits & generics** — Interface-based polymorphism with monomorphization and `where` clauses
- **Pattern matching** — Exhaustive `match` with destructuring over enums, structs, and literals
- **Async / await** — Built-in concurrency with `async`, `await`, and `spawn`
- **LLVM backend** — Compiles to native code targeting x86_64, AArch64, and WebAssembly
- **Annotations** — `@test`, `@deprecated`, `@doc`, `@inline`, and more

## Quick Start

```bash
# Build the compiler
cmake --preset default
cmake --build build

# Compile a Flux program
./build/tools/flux/flux examples/hello.fl --dump-tokens
./build/tools/flux/flux examples/hello.fl --dump-ast
```

See [README.dev.md](README.dev.md) for full build instructions and development setup.

## Language Overview

### Variables & Constants

```flux
let name: String = "Alice";         // immutable
let mut counter: Int32 = 0;         // mutable
const MAX: Int32 = 1024;            // compile-time constant
```

### Structs & Enums

```flux
struct Point {
    x: Float64,
    y: Float64,
}

enum Shape {
    Circle(Float64),
    Rectangle(Float64, Float64),
    Empty,
}
```

### Traits

```flux
trait Area {
    func area(self: &Self) -> Float64;
}

impl Area for Circle {
    func area(self: &Self) -> Float64 {
        const PI: Float64 = 3.14159265358979;
        return PI * self.radius * self.radius;
    }
}
```

### Ownership & Borrowing

```flux
func consume(buf: Buffer) -> Void { /* takes ownership */ }
func read(buf: &Buffer) -> Int32 { /* immutable borrow */ }
func modify(buf: &mut Buffer) -> Void { /* mutable borrow */ }

func main() -> Void {
    let mut buf: Buffer = createBuffer("hello");
    let size: Int32 = read(&buf);
    modify(&mut buf);
    consume(move buf);    // buf is no longer valid
}
```

### Pattern Matching

```flux
match result {
    Ok(value) => { io::println(value); }
    Err(FileError::NotFound) => { io::println("not found"); }
    Err(e) => { io::println("error"); }
}
```

### Async

```flux
async func fetchData(url: String) -> Result<String, HttpError> {
    let response: Response = await http::get(url);
    return Ok(response.body);
}
```

## Built-in Types

| Type | Description |
|------|-------------|
| `Int8` … `Int128` | Signed integers |
| `UInt8` … `UInt128` | Unsigned integers |
| `Float32`, `Float64` | IEEE 754 floats |
| `Bool` | `true` / `false` |
| `Char` | Unicode scalar |
| `String` | UTF-8 string |
| `Void` | Unit type |
| `Never` | Bottom type |
| `Option<T>` | `Some(T)` / `None` |
| `Result<T, E>` | `Ok(T)` / `Err(E)` |

## File Extension

Flux source files use the **`.fl`** extension.

## Project Structure

```
flux/
├── CMakeLists.txt          Root build configuration
├── CMakePresets.json        CMake presets (debug, release)
├── Makefile                 Convenience wrapper
├── specs.md                 Language specification
├── include/flux/            Public headers
│   ├── AST/                 Abstract syntax tree
│   ├── CodeGen/             LLVM IR generation
│   ├── Common/              Diagnostics, source locations
│   ├── Lexer/               Tokenizer
│   ├── Parser/              Recursive-descent parser
│   └── Sema/                Semantic analysis
├── lib/                     Implementation (static libraries)
│   ├── AST/
│   ├── CodeGen/
│   ├── Common/
│   ├── Lexer/
│   ├── Parser/
│   └── Sema/
├── tools/flux/              Compiler CLI driver
├── runtime/                 Flux runtime library
├── tests/                   Unit tests (GoogleTest) & lit tests
├── examples/                Example .fl programs
├── editors/vscode/          VS Code extension (syntax, snippets)
├── docs/                    VitePress documentation site
└── flux.tmLanguage.json     TextMate grammar for Flux
```

## Editor Support

A VS Code extension is included at `editors/vscode/` with:

- Syntax highlighting for all Flux keywords, types, operators, and literals
- 20+ code snippets (func, struct, enum, trait, match, etc.)
- Bracket matching, auto-closing, and code folding
- Comment toggling (`//` and `/* */`)

```bash
cd editors/vscode
npm run build
code --install-extension flux-lang-0.1.0.vsix
```

## Documentation

Full documentation lives in `docs/` and is built with [VitePress](https://vitepress.dev):

```bash
cd docs
npm install
npm run dev
```

## License

MIT
