#pragma once

#include <memory>
#include <string>
#include <vector>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class MangaLibraryActivity final : public Activity {
 private:
  ButtonNavigator buttonNavigator;
  std::vector<std::string> mangaFiles;
  std::string currentPath;
  int selectorIndex = 0;
  bool loading = false;

  void loadManga();
  void openSelected();

 public:
  explicit MangaLibraryActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("MangaLibrary", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};
