# Operators

## Arithmetic

| Operator | Description    | Example        |
|----------|----------------|----------------|
| `+`      | Addition       | `a + b`        |
| `-`      | Subtraction    | `a - b`        |
| `*`      | Multiplication | `a * b`        |
| `/`      | Division       | `a / b`        |
| `%`      | Modulus        | `a % b`        |

## Comparison

| Operator | Description        | Example        |
|----------|--------------------|----------------|
| `==`     | Equal              | `a == b`       |
| `!=`     | Not equal          | `a != b`       |
| `<`      | Less than          | `a < b`        |
| `<=`     | Less or equal      | `a <= b`       |
| `>`      | Greater than       | `a > b`        |
| `>=`     | Greater or equal   | `a >= b`       |

## Logical

| Operator | Description | Example           |
|----------|-------------|-------------------|
| `and`    | Logical AND | `a and b`         |
| `or`     | Logical OR  | `a or b`          |
| `not`    | Logical NOT | `not a`           |

## Bitwise

| Operator | Description  | Example        |
|----------|--------------|----------------|
| `&`      | Bitwise AND  | `a & b`        |
| `\|`     | Bitwise OR   | `a \| b`       |
| `^`      | Bitwise XOR  | `a ^ b`        |
| `~`      | Bitwise NOT  | `~a`           |
| `<<`     | Shift left   | `a << 2`       |
| `>>`     | Shift right  | `a >> 2`       |

## Assignment

| Operator | Description              |
|----------|--------------------------|
| `=`      | Assign                   |
| `+=`     | Add and assign           |
| `-=`     | Subtract and assign      |
| `*=`     | Multiply and assign      |
| `/=`     | Divide and assign        |
| `%=`     | Modulus and assign       |
| `&=`     | Bitwise AND and assign   |
| `\|=`    | Bitwise OR and assign    |
| `^=`     | Bitwise XOR and assign   |

## Other

| Operator | Description              | Example          |
|----------|--------------------------|------------------|
| `->`     | Return type arrow        | `func f() -> T`  |
| `=>`     | Match arm arrow          | `x => { ... }`   |
| `::`     | Path separator           | `std::io`         |
| `..`     | Range                    | `0..10`           |
| `...`    | Variadic / spread        |                   |
| `?`      | Error propagation        | `result?`         |
| `&`      | Immutable borrow         | `&value`          |
| `&mut`   | Mutable borrow           | `&mut value`      |
