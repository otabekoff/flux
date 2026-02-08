# Control Flow

## If / Else

```flux
func checkAge(age: Int32) -> String {
    if age >= 18 {
        return "Adult";
    } else if age >= 13 {
        return "Teenager";
    } else {
        return "Child";
    }
}
```

## Match

Exhaustive pattern matching over values:

```flux
func describe(x: Int32) -> String {
    match x {
        0 => { return "zero"; }
        1 => { return "one"; }
        _ => { return "other"; }
    }
}
```

### Enum Matching

```flux
enum Color {
    Red,
    Green,
    Blue,
    Custom(Int32, Int32, Int32),
}

func toHex(c: Color) -> String {
    match c {
        Color::Red => { return "#FF0000"; }
        Color::Green => { return "#00FF00"; }
        Color::Blue => { return "#0000FF"; }
        Color::Custom(r, g, b) => { return formatRGB(r, g, b); }
    }
}
```

## For Loop

Iterates over collections:

```flux
func sum(numbers: &Vec<Int32>) -> Int32 {
    let mut total: Int32 = 0;
    for n in numbers {
        total = total + n;
    }
    return total;
}
```

## While Loop

```flux
func countdown(start: Int32) -> Void {
    let mut n: Int32 = start;
    while n > 0 {
        io::print_int(n);
        n = n - 1;
    }
}
```

## Loop (Infinite)

```flux
func waitForInput() -> String {
    loop {
        let input: String = io::read_line();
        if input != "" {
            return input;
        }
    }
}
```

## Break and Continue

```flux
func findFirst(items: &Vec<Int32>, target: Int32) -> Int32 {
    let mut index: Int32 = 0;
    for item in items {
        if item == target {
            break;
        }
        index = index + 1;
    }
    return index;
}
```
