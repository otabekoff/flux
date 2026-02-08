// ============================================================
// Flux — Ownership, Borrowing, and Error Handling
// ============================================================

module ownership;

import std::io;

// -----------------------------------------------------------------------
// Ownership & move semantics
// -----------------------------------------------------------------------

struct Buffer {
    data: String,
    size: Int32,
}

func createBuffer(content: String) -> Buffer {
    return Buffer {
        data: content,
        size: 1024,
    };
}

func consumeBuffer(buf: Buffer) -> Void {
    // `buf` is moved in — this function takes ownership
    io::println(buf.data);
    // `buf` is dropped at end of scope
}

func borrowBuffer(buf: &Buffer) -> Int32 {
    // Immutable borrow — read-only access
    return buf.size;
}

func modifyBuffer(buf: &mut Buffer) -> Void {
    // Mutable borrow — can modify
    buf.size = buf.size + 512;
}

// -----------------------------------------------------------------------
// Result / Option types for error handling
// -----------------------------------------------------------------------

enum FileError {
    NotFound,
    PermissionDenied,
    IOError(String),
}

func readFile(path: String) -> Result<String, FileError> {
    if path == "" {
        return Err(FileError::NotFound);
    }
    return Ok("file contents here");
}

func findUser(id: Int32) -> Option<String> {
    if id == 1 {
        return Some("Alice");
    }
    return None;
}

// -----------------------------------------------------------------------
// Using Result with match
// -----------------------------------------------------------------------

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
            io::println("IO error:");
            io::println(msg);
        }
    }
}

// -----------------------------------------------------------------------
// Entry point
// -----------------------------------------------------------------------

func main() -> Void {
    // Ownership example
    let mut buf: Buffer = createBuffer("hello");
    let size: Int32 = borrowBuffer(&buf);    // immutable borrow
    modifyBuffer(&mut buf);                   // mutable borrow
    consumeBuffer(move buf);                  // move ownership

    // Error handling example
    processFile("test.txt");

    // Option example
    let user: Option<String> = findUser(1);
    match user {
        Some(name) => io::println(name),
        None => io::println("User not found"),
    }
}
