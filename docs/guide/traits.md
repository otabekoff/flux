# Traits & Generics

## Traits

Traits define shared behavior as a set of method signatures:

```flux
trait Display {
    func to_string(self: &Self) -> String;
}

trait Area {
    func area(self: &Self) -> Float64;
}
```

## Implementing Traits

```flux
struct Circle {
    radius: Float64,
}

impl Area for Circle {
    func area(self: &Self) -> Float64 {
        const PI: Float64 = 3.14159265358979;
        return PI * self.radius * self.radius;
    }
}

impl Display for Circle {
    func to_string(self: &Self) -> String {
        return "Circle";
    }
}
```

## Generics

Functions and types can be parameterized:

```flux
func max<T: Ord>(a: T, b: T) -> T {
    if a > b {
        return a;
    } else {
        return b;
    }
}

struct Pair<A, B> {
    first: A,
    second: B,
}
```

## Where Clauses

Complex constraints use `where`:

```flux
func print_all<T>(items: &Vec<T>) -> Void where T: Display {
    for item in items {
        io::println(item.to_string());
    }
}

func compare_and_show<T>(a: T, b: T) -> Void where T: Ord + Display {
    let bigger: T = max(a, b);
    io::println(bigger.to_string());
}
```

## Trait Bounds

Multiple bounds with `+`:

```flux
func serialize<T: Serialize + Clone>(value: T) -> Vec<UInt8> {
    let copy: T = value.clone();
    return copy.to_bytes();
}
```
