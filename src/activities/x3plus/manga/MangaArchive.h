#pragma once

#include <HalStorage.h>

#include <cstddef>
#include <cstdint>
#include <string>

class MangaArchive {
 public:
  struct PageInfo {
    std::string path;
    std::string extension;
  };

  explicit MangaArchive(std::string archivePath);
  ~MangaArchive();

  bool open();
  void close();
  bool isOpen() const { return open_; }
  const std::string& archivePath() const { return archivePath_; }
  const std::string& cachePath() const { return cachePath_; }
  std::size_t pageCount() const { return pageCount_; }
  bool getPage(std::size_t index, PageInfo& page);
  bool materializePage(std::size_t index, std::string& bmpPath, int maxWidth, int maxHeight);

  static bool isSupportedArchive(const std::string& path);

 private:
  std::string archivePath_;
  std::string cachePath_;
  HalFile indexFile_;
  std::size_t pageCount_ = 0;
  std::size_t cursorIndex_ = 0;
  bool cursorValid_ = false;
  bool open_ = false;

  bool ensureCacheDirectory();
  bool buildIndex();
  bool loadIndex();
  bool seekToPage(std::size_t index, std::string& path);
  static bool isImagePath(const std::string& path);
  static std::string extensionOf(const std::string& path);
  static std::string cacheKey(const std::string& path);
  bool extractToFile(const std::string& entryPath, const std::string& outputPath);
  bool convertImageToBmp(const std::string& inputPath, const std::string& extension,
                         const std::string& outputPath, int maxWidth, int maxHeight);
};
