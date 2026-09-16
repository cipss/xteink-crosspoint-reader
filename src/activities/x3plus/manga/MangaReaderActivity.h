#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "activities/Activity.h"

class MangaArchive;
class MangaProgress;

class MangaReaderActivity final : public Activity {
 private:
  std::unique_ptr<MangaArchive> archive;
  std::unique_ptr<MangaProgress> progress;
  std::string archivePath;
  std::string currentBmpPath;
  std::size_t pageIndex = 0;
  bool ready = false;
  bool showControls = false;
  bool error = false;
  std::string errorMessage;

  void moveNext();
  void movePrevious();
  void saveProgress();
  bool renderCurrentPage();

 public:
  explicit MangaReaderActivity(GfxRenderer& renderer, MappedInputManager& mappedInput, std::string path)
      : Activity("MangaReader", renderer, mappedInput), archivePath(std::move(path)) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};
