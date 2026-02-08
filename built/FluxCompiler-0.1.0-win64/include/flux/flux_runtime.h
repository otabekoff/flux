#pragma once

/// Flux Runtime Library
///
/// Minimal runtime support for Flux programs:
/// - Panic / abort with message
/// - Basic memory allocation wrappers
/// - String support functions
/// - Print functions (bound to C stdio)

#include <cstddef>
#include <cstdint>

#ifdef _WIN32
#ifdef FLUX_RUNTIME_EXPORTS
#define FLUX_RT_API __declspec(dllexport)
#else
#define FLUX_RT_API __declspec(dllimport)
#endif
#else
#define FLUX_RT_API __attribute__((visibility("default")))
#endif

extern "C" {

// -----------------------------------------------------------------------
// Panic & abort
// -----------------------------------------------------------------------

/// Abort with a message, file, and line info.
[[noreturn]]
FLUX_RT_API void flux_panic(const char *message, const char *file,
                            int32_t line);

/// Runtime assertion. Panics if condition is false.
FLUX_RT_API void flux_assert(bool condition, const char *message,
                             const char *file, int32_t line);

// -----------------------------------------------------------------------
// Memory
// -----------------------------------------------------------------------

/// Allocate `size` bytes, returns null on failure.
FLUX_RT_API void *flux_alloc(size_t size);

/// Allocate and zero `count * size` bytes.
FLUX_RT_API void *flux_alloc_zeroed(size_t count, size_t size);

/// Reallocate a block. Returns null on failure.
FLUX_RT_API void *flux_realloc(void *ptr, size_t newSize);

/// Free a block.
FLUX_RT_API void flux_free(void *ptr);

// -----------------------------------------------------------------------
// I/O
// -----------------------------------------------------------------------

/// Print a string to stdout (no newline).
FLUX_RT_API void flux_print(const char *str);

/// Print a string to stdout followed by newline.
FLUX_RT_API void flux_println(const char *str);

/// Print an integer to stdout.
FLUX_RT_API void flux_print_int(int64_t value);

/// Print a float to stdout.
FLUX_RT_API void flux_print_float(double value);

/// Print a boolean to stdout.
FLUX_RT_API void flux_print_bool(bool value);

// -----------------------------------------------------------------------
// String helpers
// -----------------------------------------------------------------------

/// Get the length of a Flux string.
FLUX_RT_API size_t flux_strlen(const char *str);

/// Concatenate two strings. Caller must free the result.
FLUX_RT_API char *flux_strcat(const char *a, const char *b);

/// Compare two strings. Returns 0 if equal.
FLUX_RT_API int32_t flux_strcmp(const char *a, const char *b);

} // extern "C"
