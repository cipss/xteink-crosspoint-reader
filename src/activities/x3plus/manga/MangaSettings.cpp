#include "MangaSettings.h"

#include <HalStorage.h>

#include <cstdlib>
#include <cstring>
#include <string>

namespace {

bool parseBool(const char* value, bool fallback) {
  if (!value) return fallback;
  return std::string(value) == "1" || std::string(value) == "true";
}

}  // namespace

namespace X3Plus {

MangaSettings MangaSettings::load(const std::string& path) {
  MangaSettings settings;
  HalFile file;
  if (!Storage.openFileForRead("MANGA", path.c_str(), file)) return settings;

  char buffer[512] = {};
  const size_t read = file.read(buffer, sizeof(buffer) - 1);
  file.close();
  if (read == 0) return settings;
  buffer[read] = '\0';

  char* line = buffer;
  while (line && *line) {
    char* next = std::strchr(line, '\n');
    if (next) *next++ = '\0';
    char* eq = std::strchr(line, '=');
    if (eq) {
      *eq++ = '\0';
      if (std::strcmp(line, "rtl") == 0) settings.rtl = parseBool(eq, settings.rtl);
      else if (std::strcmp(line, "prefetchNext") == 0) settings.prefetchNext = parseBool(eq, settings.prefetchNext);
      else if (std::strcmp(line, "keepPreviousCache") == 0) settings.keepPreviousCache = parseBool(eq, settings.keepPreviousCache);
      else if (std::strcmp(line, "fitMode") == 0) {
        const int mode = std::atoi(eq);
        if (mode >= 0 && mode <= static_cast<int>(MangaFitMode::Smart)) settings.fitMode = static_cast<MangaFitMode>(mode);
      }
    }
    line = next;
  }
  return settings;
}

bool MangaSettings::save(const std::string& path) const {
  HalFile file;
  if (!Storage.openFileForWrite("MANGA", path.c_str(), file)) return false;
  const std::string data = "rtl=" + std::to_string(rtl ? 1 : 0) +
                           "\nfitMode=" + std::to_string(static_cast<int>(fitMode)) +
                           "\nprefetchNext=" + std::to_string(prefetchNext ? 1 : 0) +
                           "\nkeepPreviousCache=" + std::to_string(keepPreviousCache ? 1 : 0) + "\n";
  const bool ok = file.write(data.data(), data.size()) == data.size();
  file.close();
  return ok;
}

}  // namespace X3Plus
