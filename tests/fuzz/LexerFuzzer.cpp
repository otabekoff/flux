#include <cstdint>
#include <flux/Common/Diagnostics.h>
#include <flux/Lexer/Lexer.h>
#include <stddef.h>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0)
    return 0;

  std::string source(reinterpret_cast<const char *>(Data), Size);

  // We use a dummy diagnostics engine that doesn't print to stderr to keep the
  // fuzzer fast
  flux::DiagnosticsEngine diags;
  flux::Lexer lexer(source, "fuzz.fl", diags);

  try {
    auto tokens = lexer.tokenize();
  } catch (...) {
    // We don't want the fuzzer to stop on expected exceptions,
    // only on memory errors/crashes which LibFuzzer catches automatically.
  }

  return 0;
}
