# Pattern Matching

Flux's `match` expression provides exhaustive, destructuring pattern matching.

## Basic Match

```flux
func classify(n: Int32) -> String {
    match n {
        0 => { return "zero"; }
        1 => { return "one"; }
        _ => { return "many"; }
    }
}
```

## Enum Destructuring

```flux
enum Message {
    Quit,
    Echo(String),
    Move(Int32, Int32),
    Color(Int32, Int32, Int32),
}

func handle(msg: Message) -> Void {
    match msg {
        Message::Quit => {
            io::println("Quitting");
        }
        Message::Echo(text) => {
            io::println(text);
        }
        Message::Move(x, y) => {
            io::print_int(x);
            io::print_int(y);
        }
        Message::Color(r, g, b) => {
            io::print_int(r);
        }
    }
}
```

## Option & Result Matching

```flux
func processUser(id: Int32) -> Void {
    let user: Option<User> = findUser(id);

    match user {
        Some(u) => {
            io::println(u.name);
        }
        None => {
            io::println("User not found");
        }
    }
}

func loadConfig(path: String) -> Void {
    let result: Result<Config, IOError> = readConfig(path);

    match result {
        Ok(config) => {
            applyConfig(config);
        }
        Err(IOError::NotFound) => {
            io::println("Config file not found, using defaults");
        }
        Err(e) => {
            io::println("Failed to load config");
        }
    }
}
```

## Exhaustiveness

The compiler ensures every pattern is covered. Missing cases produce a compile error:

```flux
enum Direction { North, South, East, West }

func toAngle(d: Direction) -> Float64 {
    match d {
        Direction::North => { return 0.0; }
        Direction::East  => { return 90.0; }
        // ERROR: non-exhaustive — missing South, West
    }
}
```

## Wildcard

Use `_` to match anything:

```flux
match value {
    0 => { /* handle zero */ }
    _ => { /* everything else */ }
}
```
