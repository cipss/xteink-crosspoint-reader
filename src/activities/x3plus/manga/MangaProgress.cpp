#include "MangaProgress.h"

#include <HalStorage.h>
#include <Logging.h>

namespace {
constexpr uint32_t MAGIC = 0x58334D50;  // X3MP
constexpr uint16_t VERSION = 1;

struct ProgressRecord {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t pageIndex;
};
}  // namespace

bool MangaProgress::load(uint32_t& pageIndex) const {
  pageIndex = 0;
  const std::string path = cachePath_ + "/progress.bin";

  HalFile file;
  if (!Storage.openFileForRead("MANGA", path.c_str(), file)) return false;

  ProgressRecord record{};
  const bool ok = file.read(&record, sizeof(record)) == sizeof(record) && record.magic == MAGIC && record.version == VERSION;
  file.close();
  if (!ok) return false;

  pageIndex = record.pageIndex;
  return true;
}

bool MangaProgress::save(uint32_t pageIndex) const {
  const std::string path = cachePath_ + "/progress.bin";
  HalFile file;
  if (!Storage.openFileForWrite("MANGA", path.c_str(), file)) {
    LOG_ERR("MANGA", "Failed to write progress: %s", path.c_str());
    return false;
  }

  const ProgressRecord record{MAGIC, VERSION, 0, pageIndex};
  const bool ok = file.write(&record, sizeof(record)) == sizeof(record);
  file.close();
  return ok;
}

bool MangaProgress::clear() const {
  const std::string path = cachePath_ + "/progress.bin";
  if (!Storage.exists(path.c_str())) return true;
  return Storage.remove(path.c_str());
}
