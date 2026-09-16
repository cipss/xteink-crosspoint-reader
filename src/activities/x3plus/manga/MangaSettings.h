#pragma once

#include <cstdint>
#include <string>

namespace X3Plus {

enum class MangaFitMode : uint8_t {
  FitPage = 0,
  FitWidth,
  FitHeight,
  Smart,
};

struct MangaSettings {
  bool rtl = true;
  MangaFitMode fitMode = MangaFitMode::Smart;
  bool prefetchNext = true;
  bool keepPreviousCache = true;

  static MangaSettings load(const std::string& path);
  bool save(const std::string& path) const;
};

}  // namespace X3Plus
