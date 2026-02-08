# Variables & Types

Flux is explicitly typed — every variable, parameter, and return value has a stated type.

## Let Bindings (Immutable)

```flux
let name: String = "Alice";
let age: Int32 = 30;
let pi: Float64 = 3.14159;
let active: Bool = true;
```

Immutable bindings cannot be reassigned:

```flux
let x: Int32 = 10;
// x = 20;  // ERROR: cannot assign to immutable binding
```

## Mutable Bindings

Use `mut` for mutable variables:

```flux
let mut counter: Int32 = 0;
counter = counter + 1;  // OK
```

## Constants

Compile-time constants use `const`:

```flux
const MAX_SIZE: Int32 = 1024;
const PI: Float64 = 3.14159265358979;
```

## Built-in Types

### Integers

| Type | Size | Range |
|------|------|-------|
| `Int8` | 8-bit | -128 to 127 |
| `Int16` | 16-bit | -32,768 to 32,767 |
| `Int32` | 32-bit | -2³¹ to 2³¹-1 |
| `Int64` | 64-bit | -2⁶³ to 2⁶³-1 |
| `Int128` | 128-bit | -2¹²⁷ to 2¹²⁷-1 |
| `UInt8` | 8-bit | 0 to 255 |
| `UInt16` | 16-bit | 0 to 65,535 |
| `UInt32` | 32-bit | 0 to 2³²-1 |
| `UInt64` | 64-bit | 0 to 2⁶⁴-1 |
| `UInt128` | 128-bit | 0 to 2¹²⁸-1 |

### Floating Point

| Type | Size | Precision |
|------|------|-----------|
| `Float32` | 32-bit | ~7 digits |
| `Float64` | 64-bit | ~15 digits |

### Other Primitives

| Type | Description |
|------|-------------|
| `Bool` | `true` or `false` |
| `Char` | Unicode scalar value |
| `String` | UTF-8 string |
| `Void` | No value (unit type) |
| `Never` | Never returns (bottom type) |

## Integer Literals

```flux
let decimal: Int32 = 1_000_000;
let hex: Int32 = 0xFF;
let binary: Int32 = 0b1010_0011;
let octal: Int32 = 0o77;
```

## Type Casting

Explicit casting with `as`:

```flux
let x: Int32 = 42;
let y: Float64 = x as Float64;
let z: Int64 = x as Int64;
```

No implicit conversions exist in Flux — every type conversion must be explicit.
