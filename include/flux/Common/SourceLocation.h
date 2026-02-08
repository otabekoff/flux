#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace flux {

/// Represents a position in a source file.
struct SourceLocation {
  std::string_view filename;
  uint32_t line = 1;
  uint32_t column = 1;
  uint32_t offset = 0; // byte offset into the source buffer

  static SourceLocation unknown() {
    return SourceLocation{"<unknown>", 0, 0, 0};
  }

  bool isValid() const { return line > 0 && column > 0; }

  std::string toString() const;
};

/// A range in the source file [begin, end).
struct SourceRange {
  SourceLocation begin;
  SourceLocation end;

  bool isValid() const { return begin.isValid() && end.isValid(); }
};

/// Manages source file content for the compiler.
/// Owns the source text and provides location lookups.
class SourceManager {
public:
  /// Load a source file from disk. Returns the file ID (index).
  uint32_t loadFile(const std::string &path);

  /// Load from an in-memory string (for tests).
  uint32_t loadFromString(const std::string &name, std::string content);

  /// Get the full source text for a file.
  std::string_view getSource(uint32_t fileId) const;

  /// Get the filename for a file.
  std::string_view getFilename(uint32_t fileId) const;

  /// Convert byte offset to line/column.
  SourceLocation getLocation(uint32_t fileId, uint32_t offset) const;

private:
  struct FileEntry {
    std::string filename;
    std::string content;
    std::vector<uint32_t> lineOffsets; // byte offset of each line start
  };

  std::vector<FileEntry> files_;

  void computeLineOffsets(FileEntry &entry);
};

} // namespace flux
