# Ownership & Borrowing

Flux provides memory safety without garbage collection through ownership rules enforced at compile time.

## Rules

1. Every value has exactly **one owner**.
2. When the owner goes out of scope, the value is **dropped**.
3. Ownership can be **moved** to a new binding.
4. Values can be **borrowed** immutably (`&`) or mutably (`&mut`).
5. You can have **many** immutable borrows OR **one** mutable borrow — never both.

## Move Semantics

```flux
func main() -> Void {
    let buf: Buffer = createBuffer("hello");
    consumeBuffer(move buf);

    // buf is no longer valid here — it was moved
    // io::println(buf.data);  // ERROR: use of moved value
}

func consumeBuffer(buf: Buffer) -> Void {
    io::println(buf.data);
    // buf is dropped at end of scope
}
```

## Immutable Borrow (`&`)

Read-only access without taking ownership:

```flux
func printBuffer(buf: &Buffer) -> Void {
    io::println(buf.data);
    // Cannot modify buf here
}

func main() -> Void {
    let buf: Buffer = createBuffer("hello");
    printBuffer(&buf);     // borrow
    printBuffer(&buf);     // can borrow again — still valid
}
```

## Mutable Borrow (`&mut`)

Exclusive read-write access:

```flux
func grow(buf: &mut Buffer) -> Void {
    buf.size = buf.size + 512;
}

func main() -> Void {
    let mut buf: Buffer = createBuffer("hello");
    grow(&mut buf);      // mutable borrow
    io::print_int(buf.size);
}
```

## Drop

Values are dropped when they go out of scope. You can also drop explicitly:

```flux
func example() -> Void {
    let buf: Buffer = createBuffer("temp");
    // ... use buf ...
    drop(buf);  // explicit drop — buf is freed here
}
```
