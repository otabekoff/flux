# Shapes — Traits, Enums & Generics

Demonstrates structs, enums with data, traits, impl blocks, pattern matching, and generics.

```flux
module shapes;

import std::io;

// --- Structs ---

struct Point {
    x: Float64,
    y: Float64,
}

struct Circle {
    center: Point,
    radius: Float64,
}

struct Rectangle {
    origin: Point,
    width: Float64,
    height: Float64,
}

// --- Enum with variants ---

enum Shape {
    CircleShape(Circle),
    RectShape(Rectangle),
    Line(Point, Point),
    Empty,
}

// --- Trait ---

trait Area {
    func area(self: &Self) -> Float64;
}

// --- Impl blocks ---

impl Area for Circle {
    func area(self: &Self) -> Float64 {
        const PI: Float64 = 3.14159265358979;
        return PI * self.radius * self.radius;
    }
}

impl Area for Rectangle {
    func area(self: &Self) -> Float64 {
        return self.width * self.height;
    }
}

// --- Pattern matching ---

func describeShape(s: &Shape) -> String {
    match s {
        Shape::CircleShape(c) => { return "Circle"; }
        Shape::RectShape(r)   => { return "Rectangle"; }
        Shape::Line(a, b)     => { return "Line"; }
        Shape::Empty          => { return "Empty"; }
    }
}

// --- Generic function ---

func max<T: Ord>(a: T, b: T) -> T {
    if a > b {
        return a;
    } else {
        return b;
    }
}

// --- Entry point ---

func main() -> Void {
    let origin: Point = Point { x: 0.0, y: 0.0 };
    let circle: Circle = Circle { center: origin, radius: 5.0 };
    let rect: Rectangle = Rectangle {
        origin: origin,
        width: 10.0,
        height: 20.0,
    };

    let circleArea: Float64 = circle.area();
    let rectArea: Float64 = rect.area();

    io::println("Circle area:");
    io::print_float(circleArea);
    io::println("");

    io::println("Rectangle area:");
    io::print_float(rectArea);
    io::println("");

    let bigger: Int32 = max(42, 17);
    io::print_int(bigger);
}
```
