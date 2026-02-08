# Introduction

**Flux** is a modern, production-ready, statically typed programming language designed for systems programming. It emphasizes explicitness — every type is stated, every allocation is visible, and there are no hidden conversions or copies.

## Philosophy

- **Always explicit** — No implicit type conversions, no hidden copies, no magic.
- **Ownership & borrowing** — Memory safety without garbage collection.
- **Traits over inheritance** — Composition and interfaces, not class hierarchies.
- **Pattern matching** — Exhaustive, expressive control flow.
- **Native performance** — Compiles to machine code via LLVM.

## Key Features

| Feature              | Description                                         |
|----------------------|-----------------------------------------------------|
| Static typing        | Every variable and parameter has an explicit type    |
| Ownership            | Move semantics with `move`, `&` borrow, `&mut`      |
| Traits & Generics    | Interface-based polymorphism with monomorphization   |
| Pattern matching     | `match` with destructuring, guards, exhaustiveness   |
| Async/Await          | Built-in concurrency with `async`, `await`, `spawn` |
| LLVM backend         | x86_64, AArch64, WebAssembly targets                |
| Annotations          | `@test`, `@deprecated`, `@doc`, `@inline`           |

## Quick Example

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

## Next Steps

- [Installation](/guide/installation) — Set up the Flux compiler
- [Hello World](/guide/hello-world) — Write your first Flux program
- [Variables & Types](/guide/variables) — Learn the type system
