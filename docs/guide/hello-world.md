# Hello World

Let's write your first Flux program.

## Create a file

Create a file called `hello.fl`:

```flux
module hello;

import std::io;

func main() -> Void {
    let message: String = "Hello, Flux!";
    io::println(message);
}
```

## Breakdown

| Line | Meaning |
|------|---------|
| `module hello;` | Declares this file as the `hello` module |
| `import std::io;` | Imports the standard I/O module |
| `func main() -> Void` | Entry point — every Flux program starts here |
| `let message: String` | Immutable binding with explicit type |
| `io::println(message)` | Prints to stdout via the `std::io` module |

## Compile and run

```bash
flux hello.fl -o hello
./hello
```

Output:
```
Hello, Flux!
```

## Adding more

```flux
module hello;

import std::io;

func main() -> Void {
    let message: String = "Hello, Flux!";
    io::println(message);

    // Mutable binding
    let mut counter: Int32 = 0;
    counter = counter + 1;

    // Constants
    const PI: Float64 = 3.14159265358979;

    // Function call
    let result: Int32 = add(10, 20);
    io::print_int(result);
}

func add(a: Int32, b: Int32) -> Int32 {
    return a + b;
}
```

Every variable has an explicit type. Every function declares its parameter types and return type. Nothing is hidden.
