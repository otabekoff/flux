---
layout: home

hero:
  name: Flux
  text: A Modern Systems Language
  tagline: Production-ready, explicitly typed, with ownership & borrowing — no hidden stuff.
  actions:
    - theme: brand
      text: Get Started
      link: /guide/
    - theme: alt
      text: View on GitHub
      link: https://github.com/flux-lang/flux

features:
  - title: Explicit by Design
    details: Every type is stated, every allocation is visible. Flux enforces clarity — no implicit conversions, no hidden copies.
    icon: 🎯
  - title: Ownership & Borrowing
    details: Memory safety without a garbage collector. Move semantics, immutable & mutable borrows, and deterministic drop.
    icon: 🔒
  - title: Traits & Generics
    details: Powerful trait-based polymorphism with monomorphized generics and where clauses for precise constraints.
    icon: 🧩
  - title: Pattern Matching
    details: Exhaustive match expressions over enums, structs, and literals with destructuring and guard clauses.
    icon: 🔀
  - title: Async / Await
    details: First-class async support with spawn for concurrent tasks — built into the language, not bolted on.
    icon: ⚡
  - title: LLVM Backend
    details: Compiles to native code via LLVM with support for x86_64, AArch64, and WebAssembly targets.
    icon: ⚙️
---
