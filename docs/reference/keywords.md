# Keywords

All reserved keywords in the Flux language.

## Declarations

| Keyword  | Description                         |
|----------|-------------------------------------|
| `module` | Module declaration                  |
| `import` | Import from another module          |
| `func`   | Function definition                 |
| `let`    | Variable binding                    |
| `mut`    | Mutable modifier                    |
| `const`  | Constant declaration                |
| `struct` | Struct type definition              |
| `class`  | Class type definition               |
| `enum`   | Enum type definition                |
| `trait`  | Trait (interface) definition        |
| `impl`   | Implementation block                |
| `type`   | Type alias                          |
| `use`    | Use declaration                     |

## Control Flow

| Keyword    | Description                  |
|------------|------------------------------|
| `if`       | Conditional branch           |
| `else`     | Alternative branch           |
| `match`    | Pattern matching expression  |
| `for`      | For-in loop                  |
| `while`    | While loop                   |
| `loop`     | Infinite loop                |
| `break`    | Break out of loop            |
| `continue` | Skip to next iteration       |
| `return`   | Return from function         |
| `in`       | Collection iteration         |

## Ownership & Borrowing

| Keyword | Description                |
|---------|----------------------------|
| `move`  | Transfer ownership         |
| `ref`   | Reference                  |
| `drop`  | Explicit deallocation      |

## Concurrency

| Keyword | Description              |
|---------|--------------------------|
| `async` | Async function modifier  |
| `await` | Await a future           |
| `spawn` | Spawn a concurrent task  |

## Visibility

| Keyword   | Description                 |
|-----------|-----------------------------|
| `pub`     | Public visibility (short)   |
| `public`  | Public visibility           |
| `private` | Private visibility          |

## Safety

| Keyword  | Description            |
|----------|------------------------|
| `unsafe` | Unsafe code block      |

## Logic & Type Operations

| Keyword | Description            |
|---------|------------------------|
| `and`   | Logical AND            |
| `or`    | Logical OR             |
| `not`   | Logical NOT            |
| `as`    | Type cast              |
| `is`    | Type check             |
| `where` | Generic constraints    |

## Other

| Keyword  | Description                  |
|----------|------------------------------|
| `self`   | Current instance             |
| `Self`   | Current type                 |
| `Void`   | No return value              |
| `true`   | Boolean true literal         |
| `false`  | Boolean false literal        |
| `panic`  | Abort with message           |
| `assert` | Runtime assertion            |

## Annotations

| Annotation    | Description                  |
|---------------|------------------------------|
| `@doc`        | Documentation annotation     |
| `@deprecated` | Mark as deprecated           |
| `@test`       | Mark as test function        |
| `@inline`     | Inline hint                  |
