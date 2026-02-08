#include "flux_runtime.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" {

// -----------------------------------------------------------------------
// Panic & abort
// -----------------------------------------------------------------------

[[noreturn]]
void flux_panic(const char *message, const char *file, int32_t line) {
  std::fprintf(stderr, "PANIC at %s:%d: %s\n", file, line, message);
  std::fflush(stderr);
  std::abort();
}

void flux_assert(bool condition, const char *message, const char *file,
                 int32_t line) {
  if (!condition) {
    flux_panic(message, file, line);
  }
}

// -----------------------------------------------------------------------
// Memory
// -----------------------------------------------------------------------

void *flux_alloc(size_t size) {
  void *ptr = std::malloc(size);
  if (!ptr && size > 0) {
    flux_panic("allocation failed", __FILE__, __LINE__);
  }
  return ptr;
}

void *flux_alloc_zeroed(size_t count, size_t size) {
  void *ptr = std::calloc(count, size);
  if (!ptr && count > 0 && size > 0) {
    flux_panic("allocation failed", __FILE__, __LINE__);
  }
  return ptr;
}

void *flux_realloc(void *ptr, size_t newSize) {
  void *result = std::realloc(ptr, newSize);
  if (!result && newSize > 0) {
    flux_panic("reallocation failed", __FILE__, __LINE__);
  }
  return result;
}

void flux_free(void *ptr) { std::free(ptr); }

// -----------------------------------------------------------------------
// I/O
// -----------------------------------------------------------------------

void flux_print(const char *str) {
  std::fputs(str, stdout);
  std::fflush(stdout);
}

void flux_println(const char *str) {
  std::puts(str);
  std::fflush(stdout);
}

void flux_print_int(int64_t value) {
  std::printf("%lld", static_cast<long long>(value));
  std::fflush(stdout);
}

void flux_print_float(double value) {
  std::printf("%g", value);
  std::fflush(stdout);
}

void flux_print_bool(bool value) {
  std::fputs(value ? "true" : "false", stdout);
  std::fflush(stdout);
}

// -----------------------------------------------------------------------
// String helpers
// -----------------------------------------------------------------------

size_t flux_strlen(const char *str) { return std::strlen(str); }

char *flux_strcat(const char *a, const char *b) {
  size_t lenA = std::strlen(a);
  size_t lenB = std::strlen(b);
  char *result = static_cast<char *>(flux_alloc(lenA + lenB + 1));
  std::memcpy(result, a, lenA);
  std::memcpy(result + lenA, b, lenB + 1); // includes null terminator
  return result;
}

int32_t flux_strcmp(const char *a, const char *b) {
  return static_cast<int32_t>(std::strcmp(a, b));
}

} // extern "C"
