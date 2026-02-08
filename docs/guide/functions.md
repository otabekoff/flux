# Functions

Functions in Flux always declare explicit parameter types and return types.

## Basic Functions

```flux
func add(a: Int32, b: Int32) -> Int32 {
    return a + b;
}

func greet(name: String) -> Void {
    io::println("Hello, ");
    io::println(name);
}
```

## Visibility

Functions are private by default. Use `pub` or `public` to export:

```flux
pub func publicApi(x: Int32) -> Int32 {
    return helperFunction(x);
}

func helperFunction(x: Int32) -> Int32 {
    return x * 2;
}
```

## Generic Functions

```flux
func max<T: Ord>(a: T, b: T) -> T {
    if a > b {
        return a;
    } else {
        return b;
    }
}

func identity<T>(value: T) -> T {
    return value;
}
```

## Where Clauses

```flux
func serialize<T>(value: T) -> String where T: Serialize + Display {
    return value.to_string();
}
```

## Async Functions

```flux
async func fetchData(url: String) -> Result<String, HttpError> {
    let response: Response = await http::get(url);
    return Ok(response.body);
}
```

## Self Parameter (Methods)

When a function is inside an `impl` block, it can take `self`:

```flux
struct Counter {
    value: Int32,
}

impl Counter {
    func new() -> Counter {
        return Counter { value: 0 };
    }

    func get(self: &Self) -> Int32 {
        return self.value;
    }

    func increment(self: &mut Self) -> Void {
        self.value = self.value + 1;
    }

    func consume(self: Self) -> Int32 {
        return self.value;
    }
}
```
