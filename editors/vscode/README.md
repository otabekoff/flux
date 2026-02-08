# Flux Language Support for VS Code

Rich language support for the **Flux** programming language.

## Features

- **Syntax Highlighting** — Full TextMate grammar covering all Flux keywords, operators, literals, types, annotations, and comments.
- **Snippets** — Quick templates for functions, structs, enums, traits, control flow, and more.
- **Bracket Matching** — Auto-closing and colorized bracket pairs for `{}`, `[]`, `()`.
- **Comment Toggling** — Line (`//`) and block (`/* */`) comment support.
- **Folding** — Code folding with region markers.
- **Indentation** — Automatic indent/dedent rules.

## Supported File Extension

| Extension | Language |
|-----------|----------|
| `.fl`     | Flux     |

## Installation

### From VSIX (local)

```bash
cd editors/vscode
npx @vscode/vsce package
code --install-extension flux-lang-0.1.0.vsix
```

### From source (development)

1. Open `editors/vscode/` in VS Code
2. Press `F5` to launch Extension Development Host
3. Open any `.fl` file to see syntax highlighting

## Snippets

| Prefix      | Description               |
|-------------|---------------------------|
| `func`      | Function definition       |
| `main`      | Main entry point          |
| `let`       | Immutable binding         |
| `letmut`    | Mutable binding           |
| `const`     | Constant declaration      |
| `struct`    | Struct definition         |
| `enum`      | Enum definition           |
| `trait`     | Trait definition          |
| `impl`      | Impl block                |
| `if`        | If statement              |
| `ife`       | If-else statement         |
| `match`     | Match expression          |
| `for`       | For-in loop               |
| `while`     | While loop                |
| `loop`      | Infinite loop             |
| `module`    | Module declaration        |
| `import`    | Import declaration        |
| `asyncfunc` | Async function            |
| `test`      | Test function             |
| `println`   | Print line                |

## License

MIT
