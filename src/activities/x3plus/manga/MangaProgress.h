#pragma once

#include <cstdint>
#include <string>
#include <utility>

class MangaProgress final {
 public:
  explicit MangaProgress(std::string cachePath) : cachePath_(std::move(cachePath)) {}

  bool load(uint32_t& pageIndex) const;
  bool save(uint32_t pageIndex) const;
  bool clear() const;

 private:
  std::string cachePath_;
};
