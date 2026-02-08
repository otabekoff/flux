# Async & Concurrency

Flux has built-in support for asynchronous programming with `async`, `await`, and `spawn`.

## Async Functions

```flux
async func fetchPage(url: String) -> Result<String, HttpError> {
    let response: Response = await http::get(url);
    return Ok(response.body);
}
```

## Await

Use `await` to suspend execution until a future completes:

```flux
async func main() -> Void {
    let page: Result<String, HttpError> = await fetchPage("https://example.com");

    match page {
        Ok(body) => io::println(body),
        Err(e)   => io::println("Request failed"),
    }
}
```

## Spawn

Launch concurrent tasks with `spawn`:

```flux
async func fetchAll() -> Void {
    let task1 = spawn fetchPage("https://api.example.com/a");
    let task2 = spawn fetchPage("https://api.example.com/b");

    let result1: Result<String, HttpError> = await task1;
    let result2: Result<String, HttpError> = await task2;
}
```

## Concurrency Rules

- Shared state across `spawn` boundaries must use `Arc` or message passing.
- The ownership system prevents data races at compile time.
- `&mut` borrows cannot be shared across concurrent tasks.
