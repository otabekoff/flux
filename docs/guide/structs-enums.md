# Structs & Enums

## Structs

Structs group named fields with explicit types:

```flux
struct Point {
    x: Float64,
    y: Float64,
}

struct User {
    name: String,
    age: Int32,
    active: Bool,
}
```

### Creating Instances

```flux
let origin: Point = Point { x: 0.0, y: 0.0 };
let user: User = User {
    name: "Alice",
    age: 30,
    active: true,
};
```

### Accessing Fields

```flux
let name: String = user.name;
let x: Float64 = origin.x;
```

### Methods via Impl

```flux
impl Point {
    func new(x: Float64, y: Float64) -> Point {
        return Point { x: x, y: y };
    }

    func distance(self: &Self, other: &Point) -> Float64 {
        let dx: Float64 = self.x - other.x;
        let dy: Float64 = self.y - other.y;
        return (dx * dx + dy * dy).sqrt();
    }
}
```

## Enums

Enums define a type with a fixed set of variants:

```flux
enum Direction {
    North,
    South,
    East,
    West,
}
```

### Variants with Data

```flux
enum Shape {
    Circle(Float64),
    Rectangle(Float64, Float64),
    Triangle(Point, Point, Point),
    Empty,
}
```

### Using Enums

```flux
let s: Shape = Shape::Circle(5.0);

match s {
    Shape::Circle(radius) => {
        io::println("Circle");
    }
    Shape::Rectangle(w, h) => {
        io::println("Rectangle");
    }
    Shape::Triangle(a, b, c) => {
        io::println("Triangle");
    }
    Shape::Empty => {
        io::println("Empty");
    }
}
```

## Option and Result

Flux provides `Option<T>` and `Result<T, E>` for safe error handling:

```flux
func find(id: Int32) -> Option<User> {
    if id == 1 {
        return Some(User { name: "Alice", age: 30, active: true });
    }
    return None;
}

func parse(input: String) -> Result<Int32, ParseError> {
    // ...
}
```
