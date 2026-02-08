# Error Handling

Flux uses `Result<T, E>` and `Option<T>` for explicit error handling — no exceptions.

## Result Type

```flux
enum FileError {
    NotFound,
    PermissionDenied,
    IOError(String),
}

func readFile(path: String) -> Result<String, FileError> {
    if path == "" {
        return Err(FileError::NotFound);
    }
    return Ok("file contents");
}
```

## Using Result with Match

```flux
func processFile(path: String) -> Void {
    let result: Result<String, FileError> = readFile(path);

    match result {
        Ok(contents) => {
            io::println(contents);
        }
        Err(FileError::NotFound) => {
            io::println("File not found");
        }
        Err(FileError::PermissionDenied) => {
            io::println("Permission denied");
        }
        Err(FileError::IOError(msg)) => {
            io::println(msg);
        }
    }
}
```

## Option Type

```flux
func findUser(id: Int32) -> Option<String> {
    if id == 1 {
        return Some("Alice");
    }
    return None;
}

func main() -> Void {
    let user: Option<String> = findUser(1);

    match user {
        Some(name) => io::println(name),
        None => io::println("User not found"),
    }
}
```

## Panic and Assert

For unrecoverable errors:

```flux
func divide(a: Int32, b: Int32) -> Int32 {
    assert(b != 0);  // aborts if false
    return a / b;
}

func unreachable() -> Never {
    panic("This should never happen");
}
```
