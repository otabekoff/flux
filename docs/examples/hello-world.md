# Hello World

A minimal Flux program.

```flux
module hello;

import std::io;

/// Entry point — every Flux program starts from `main`.
func main() -> Void {
    // Immutable binding with explicit type
    let message: String = "Hello, Flux!";
    io::println(message);

    // Mutable binding
    let mut counter: Int32 = 0;
    counter = counter + 1;

    // Constant
    const PI: Float64 = 3.14159265358979;

    // Function call
    let result: Int32 = add(10, 20);
    io::println("10 + 20 = ");
    io::print_int(result);
}

/// A simple function with explicit parameter and return types.
func add(a: Int32, b: Int32) -> Int32 {
    return a + b;
}
```

Compile and run:

```bash
flux hello.fl -o hello
./hello
```
