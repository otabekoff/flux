#include "flux/Common/SourceLocation.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace flux {

std::string SourceLocation::toString() const {
  std::string result;
  result += filename;
  result += ':';
  result += std::to_string(line);
  result += ':';
  result += std::to_string(column);
  return result;
}

uint32_t SourceManager::loadFile(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + path);
  }

  std::stringstream buffer;
  buffer << file.rdbuf();

  FileEntry entry;
  entry.filename = path;
  entry.content = buffer.str();
  computeLineOffsets(entry);

  files_.push_back(std::move(entry));
  return static_cast<uint32_t>(files_.size() - 1);
}

uint32_t SourceManager::loadFromString(const std::string &name,
                                       std::string content) {
  FileEntry entry;
  entry.filename = name;
  entry.content = std::move(content);
  computeLineOffsets(entry);

  files_.push_back(std::move(entry));
  return static_cast<uint32_t>(files_.size() - 1);
}

std::string_view SourceManager::getSource(uint32_t fileId) const {
  if (fileId >= files_.size()) {
    throw std::out_of_range("Invalid file ID");
  }
  return files_[fileId].content;
}

std::string_view SourceManager::getFilename(uint32_t fileId) const {
  if (fileId >= files_.size()) {
    throw std::out_of_range("Invalid file ID");
  }
  return files_[fileId].filename;
}

SourceLocation SourceManager::getLocation(uint32_t fileId,
                                          uint32_t offset) const {
  if (fileId >= files_.size()) {
    return SourceLocation::unknown();
  }

  const auto &entry = files_[fileId];
  if (offset >= entry.content.size()) {
    return SourceLocation::unknown();
  }

  // Binary search for the line
  const auto &offsets = entry.lineOffsets;
  uint32_t line = 0;
  uint32_t lo = 0, hi = static_cast<uint32_t>(offsets.size());
  while (lo < hi) {
    uint32_t mid = lo + (hi - lo) / 2;
    if (offsets[mid] <= offset) {
      line = mid;
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }

  uint32_t column = offset - offsets[line] + 1;

  SourceLocation loc;
  loc.filename = entry.filename;
  loc.line = line + 1; // 1-based
  loc.column = column;
  loc.offset = offset;
  return loc;
}

void SourceManager::computeLineOffsets(FileEntry &entry) {
  entry.lineOffsets.clear();
  entry.lineOffsets.push_back(0); // First line starts at offset 0

  for (uint32_t i = 0; i < entry.content.size(); ++i) {
    if (entry.content[i] == '\n') {
      entry.lineOffsets.push_back(i + 1);
    }
  }
}

} // namespace flux
