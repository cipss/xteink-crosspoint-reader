#include "MangaLibraryActivity.h"

#include <FsHelpers.h>
#include <HalStorage.h>

#include <algorithm>

#include "components/UITheme.h"
#include "fontIds.h"
#include "manga/MangaArchive.h"
#include "manga/MangaReaderActivity.h"

namespace {
constexpr char MANGA_ROOT[] = "/Manga";
}

void MangaLibraryActivity::loadManga() {
  mangaFiles.clear();
  currentPath = MANGA_ROOT;

  auto root = Storage.open(MANGA_ROOT);
  if (!root) {
    Storage.mkdir(MANGA_ROOT);
    return;
  }
  if (!root.isDirectory()) {
    root.close();
    return;
  }

  root.rewindDirectory();
  char name[256];
  for (auto entry = root.openNextFile(); entry; entry = root.openNextFile()) {
    entry.getName(name, sizeof(name));
    if (entry.isDirectory()) continue;
    const std::string filename(name);
    if (MangaArchive::isSupportedArchive(filename)) {
      mangaFiles.push_back(std::string(MANGA_ROOT) + "/" + filename);
    }
  }
  root.close();

  FsHelpers::sortFileList(mangaFiles);
  if (selectorIndex > static_cast<int>(mangaFiles.size())) selectorIndex = static_cast<int>(mangaFiles.size());
}

void MangaLibraryActivity::openSelected() {
  const int settingsIndex = static_cast<int>(mangaFiles.size());
  if (selectorIndex == settingsIndex) {
    activityManager.goToMangaSettings();
    return;
  }
  if (selectorIndex < 0 || selectorIndex >= settingsIndex) return;
  activityManager.pushActivity(std::make_unique<MangaReaderActivity>(renderer, mappedInput, mangaFiles[selectorIndex]));
}

void MangaLibraryActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  loading = true;
  loadManga();
  loading = false;
  requestUpdate();
}

void MangaLibraryActivity::onExit() {
  Activity::onExit();
  mangaFiles.clear();
}

void MangaLibraryActivity::loop() {
  const int itemCount = static_cast<int>(mangaFiles.size()) + 1;
  buttonNavigator.onNextRelease([this, itemCount] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, itemCount);
    requestUpdate();
  });
  buttonNavigator.onPreviousRelease([this, itemCount] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, itemCount);
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    openSelected();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome(HomeMenuItem::X3PLUS);
  }
}

void MangaLibraryActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  const int totalItems = static_cast<int>(mangaFiles.size()) + 1;

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, "Manga");

  const Rect content{0, metrics.topPadding + metrics.headerHeight, width,
                     height - (metrics.topPadding + metrics.headerHeight + metrics.buttonHintsHeight)};

  if (loading) {
    GUI.drawPopup(renderer, "Scanning Manga...");
  } else {
    GUI.drawList(
        renderer, content, totalItems, selectorIndex,
        [this](int index) {
          if (index == static_cast<int>(mangaFiles.size())) return std::string("Manga Settings");
          std::string name = mangaFiles[index];
          const auto pos = name.find_last_of('/');
          if (pos != std::string::npos) name = name.substr(pos + 1);
          const auto ext = name.find_last_of('.');
          if (ext != std::string::npos) name.resize(ext);
          return name;
        },
        nullptr,
        [this](int index) { return index == static_cast<int>(mangaFiles.size()) ? Settings : Book; });

    if (mangaFiles.empty()) {
      renderer.drawCenteredText(UI_10_FONT_ID, content.y + content.height - 45, "Inserisci CBZ/ZIP nella cartella /Manga");
    }
  }

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_OPEN), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
