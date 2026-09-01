#pragma once

#include <Arduino.h>
#include <Print.h>
#include <string>
#include <vector>

class HalFile;

class HalStorage {
 public:
  HalStorage() = default;
  bool begin() { return true; }
  bool ready() const { return true; }
  std::vector<String> listFiles(const char* path = "/", int maxFiles = 200) {
    (void)path;
    (void)maxFiles;
    return {};
  }
  String readFile(const char* path) {
    (void)path;
    return String();
  }
  bool readFileToStream(const char* path, Print& out, size_t chunkSize = 256) {
    (void)path;
    (void)out;
    (void)chunkSize;
    return true;
  }
  size_t readFileToBuffer(const char* path, char* buffer, size_t bufferSize, size_t maxBytes = 0) {
    (void)path;
    (void)buffer;
    (void)bufferSize;
    (void)maxBytes;
    return 0;
  }
  bool writeFile(const char* path, const String& content) {
    (void)path;
    (void)content;
    return true;
  }
  bool ensureDirectoryExists(const char* path) {
    (void)path;
    return true;
  }
  bool exists(const char* path) { return false; }
  bool mkdir(const char* path, const bool pFlag = true) { (void)path; (void)pFlag; return true; }
  bool remove(const char* path) { (void)path; return true; }
  bool rename(const char* oldPath, const char* newPath) { (void)oldPath; (void)newPath; return true; }
  bool rmdir(const char* path) { (void)path; return true; }

  static HalStorage& getInstance() {
    static HalStorage instance;
    return instance;
  }

 private:
  static HalStorage instance;
};

inline HalStorage HalStorage::instance;

#define Storage HalStorage::getInstance()
