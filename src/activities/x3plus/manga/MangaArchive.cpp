#include "MangaArchive.h"

#include <FsHelpers.h>
#include <JpegToBmpConverter.h>
#include <Logging.h>
#include <Memory.h>
#include <PngToBmpConverter.h>
#include <ZipFile.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>

namespace {
constexpr uint16_t MAX_ENTRY_PATH = 65500;
constexpr char CACHE_ROOT[] = "/.crosspoint/x3plus/manga";
constexpr char INDEX_FILE[] = "index.bin";
constexpr char TEMP_IMAGE[] = "input.bin";
constexpr uint32_t FNV_OFFSET = 2166136261u;
constexpr uint32_t FNV_PRIME = 16777619u;

uint32_t fnv1a(const std::string& value) {
  uint32_t hash = FNV_OFFSET;
  for (unsigned char c : value) {
    hash ^= c;
    hash *= FNV_PRIME;
  }
  return hash;
}

bool extensionEquals(const std::string& path, const char* ext) {
  const std::size_t extLen = std::strlen(ext);
  if (path.size() < extLen) return false;
  const std::size_t pos = path.size() - extLen;
  for (std::size_t i = 0; i < extLen; ++i) {
    if (std::tolower(static_cast<unsigned char>(path[pos + i])) !=
        std::tolower(static_cast<unsigned char>(ext[i]))) {
      return false;
    }
  }
  return true;
}
}  // namespace

MangaArchive::MangaArchive(std::string archivePath) : archivePath_(std::move(archivePath)) {}

MangaArchive::~MangaArchive() { close(); }

std::string MangaArchive::cacheKey(const std::string& path) {
  uint32_t hash = fnv1a(path);
  auto file = Storage.open(path.c_str());
  if (file) {
    hash ^= static_cast<uint32_t>(file.size());
    hash *= FNV_PRIME;
    file.close();
  }

  char buf[32];
  std::snprintf(buf, sizeof(buf), "%08x", hash);
  return std::string(CACHE_ROOT) + "/" + buf;
}

std::string MangaArchive::extensionOf(const std::string& path) {
  const auto slash = path.find_last_of('/');
  const auto dot = path.find_last_of('.');
  if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) return {};

  std::string ext = path.substr(dot);
  std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return ext;
}

bool MangaArchive::isSupportedArchive(const std::string& path) {
  return extensionEquals(path, ".cbz") || extensionEquals(path, ".zip");
}

bool MangaArchive::isImagePath(const std::string& path) {
  return extensionEquals(path, ".jpg") || extensionEquals(path, ".jpeg") || extensionEquals(path, ".png") ||
         extensionEquals(path, ".bmp");
}

bool MangaArchive::ensureCacheDirectory() {
  if (!Storage.exists(CACHE_ROOT) && !Storage.mkdir(CACHE_ROOT)) {
    LOG_ERR("MANGA", "Failed to create cache root");
    return false;
  }

  if (!Storage.exists(cachePath_.c_str()) && !Storage.mkdir(cachePath_.c_str())) {
    LOG_ERR("MANGA", "Failed to create manga cache: %s", cachePath_.c_str());
    return false;
  }
  return true;
}

bool MangaArchive::buildIndex() {
  ZipFile zip(archivePath_);
  if (!zip.open()) {
    LOG_ERR("MANGA", "Unable to open archive: %s", archivePath_.c_str());
    return false;
  }

  const std::string indexPath = cachePath_ + "/" + INDEX_FILE;
  if (Storage.exists(indexPath.c_str())) Storage.remove(indexPath.c_str());

  HalFile index;
  if (!Storage.openFileForWrite("MANGA", indexPath.c_str(), index)) {
    zip.close();
    return false;
  }

  pageCount_ = 0;
  bool writeOk = true;
  const bool enumerateOk = zip.enumerateFilePaths([&](std::string_view filePath) {
    if (!writeOk || filePath.empty() || filePath.back() == '/' || filePath.size() > MAX_ENTRY_PATH) return;

    const std::string path(filePath);
    if (!isImagePath(path)) return;

    const uint16_t len = static_cast<uint16_t>(path.size());
    if (index.write(&len, sizeof(len)) != sizeof(len) || index.write(path.data(), len) != len) {
      writeOk = false;
      return;
    }
    ++pageCount_;
  });

  index.close();
  zip.close();
  return enumerateOk && writeOk && pageCount_ > 0;
}

bool MangaArchive::loadIndex() {
  const std::string indexPath = cachePath_ + "/" + INDEX_FILE;
  if (!Storage.exists(indexPath.c_str())) return buildIndex();

  HalFile file;
  if (!Storage.openFileForRead("MANGA", indexPath.c_str(), file)) return false;

  pageCount_ = 0;
  while (file.available()) {
    uint16_t len = 0;
    if (file.read(&len, sizeof(len)) != sizeof(len) || len == 0 || len > MAX_ENTRY_PATH) {
      file.close();
      return false;
    }
    file.seekCur(len);
    ++pageCount_;
  }
  file.close();
  return pageCount_ > 0;
}

bool MangaArchive::open() {
  close();
  if (!isSupportedArchive(archivePath_)) return false;

  cachePath_ = cacheKey(archivePath_);
  if (!ensureCacheDirectory()) return false;
  if (!loadIndex()) return false;

  open_ = true;
  cursorValid_ = false;
  cursorIndex_ = 0;
  return true;
}

void MangaArchive::close() {
  if (indexFile_) indexFile_.close();
  open_ = false;
  cursorValid_ = false;
  cursorIndex_ = 0;
}

bool MangaArchive::seekToPage(std::size_t index, std::string& path) {
  if (!open_ || index >= pageCount_) return false;

  const std::string indexPath = cachePath_ + "/" + INDEX_FILE;
  HalFile file;
  if (!Storage.openFileForRead("MANGA", indexPath.c_str(), file)) return false;

  for (std::size_t i = 0; i < index; ++i) {
    uint16_t len = 0;
    if (file.read(&len, sizeof(len)) != sizeof(len) || len == 0 || len > MAX_ENTRY_PATH) {
      file.close();
      return false;
    }
    file.seekCur(len);
  }

  uint16_t len = 0;
  if (file.read(&len, sizeof(len)) != sizeof(len) || len == 0 || len > MAX_ENTRY_PATH) {
    file.close();
    return false;
  }

  std::unique_ptr<char[]> buffer = makeUniqueNoThrow<char[]>(static_cast<std::size_t>(len) + 1);
  if (!buffer) {
    LOG_ERR("MANGA", "OOM reading page index");
    file.close();
    return false;
  }

  if (file.read(buffer.get(), len) != len) {
    file.close();
    return false;
  }
  buffer[len] = '\0';
  path.assign(buffer.get(), len);
  file.close();

  cursorIndex_ = index;
  cursorValid_ = true;
  return true;
}

bool MangaArchive::getPage(std::size_t index, PageInfo& page) {
  if (!seekToPage(index, page.path)) return false;
  page.extension = extensionOf(page.path);
  return true;
}

bool MangaArchive::extractToFile(const std::string& entryPath, const std::string& outputPath) {
  ZipFile zip(archivePath_);
  HalFile output;
  if (!Storage.openFileForWrite("MANGA", outputPath.c_str(), output)) return false;

  const bool success = zip.readFileToStream(entryPath.c_str(), output, 4096);
  output.close();
  return success;
}

bool MangaArchive::convertImageToBmp(const std::string& inputPath, const std::string& extension,
                                     const std::string& outputPath, int maxWidth, int maxHeight) {
  HalFile input;
  HalFile output;
  if (!Storage.openFileForRead("MANGA", inputPath.c_str(), input)) return false;
  if (!Storage.openFileForWrite("MANGA", outputPath.c_str(), output)) {
    input.close();
    return false;
  }

  bool success = false;
  if (extension == ".jpg" || extension == ".jpeg") {
    success = JpegToBmpConverter::jpegFileToBmpStreamWithSize(input, output, maxWidth, maxHeight);
  } else if (extension == ".png") {
    success = PngToBmpConverter::pngFileToBmpStreamWithSize(input, output, maxWidth, maxHeight);
  }

  input.close();
  output.close();
  return success;
}

bool MangaArchive::materializePage(std::size_t index, std::string& bmpPath, int maxWidth, int maxHeight) {
  PageInfo page;
  if (!getPage(index, page)) return false;

  char bmpName[48];
  std::snprintf(bmpName, sizeof(bmpName), "/page_%06lu.bmp", static_cast<unsigned long>(index));
  bmpPath = cachePath_ + bmpName;
  if (Storage.exists(bmpPath.c_str())) return true;

  const std::string inputPath = cachePath_ + "/" + TEMP_IMAGE;
  if (Storage.exists(inputPath.c_str())) Storage.remove(inputPath.c_str());
  if (!extractToFile(page.path, inputPath)) {
    Storage.remove(inputPath.c_str());
    return false;
  }

  bool success = false;
  if (page.extension == ".bmp") {
    success = Storage.rename(inputPath.c_str(), bmpPath.c_str());
  } else {
    success = convertImageToBmp(inputPath, page.extension, bmpPath, maxWidth, maxHeight);
    Storage.remove(inputPath.c_str());
  }

  if (!success) Storage.remove(bmpPath.c_str());
  return success;
}
