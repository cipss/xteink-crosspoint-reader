#include "MangaReaderActivity.h"

#include <Bitmap.h>
#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>

#include "components/UITheme.h"
#include "fontIds.h"
#include "manga/MangaArchive.h"
#include "manga/MangaProgress.h"
#include "manga/MangaSettings.h"

namespace {
constexpr char MANGA_SETTINGS_PATH[] = "/.crosspoint/x3plus/manga/settings.cfg";
}

void MangaReaderActivity::onEnter() {
  Activity::onEnter();
  archive = std::make_unique<MangaArchive>(archivePath);
  if (!archive->open()) {
    error = true;
    errorMessage = "Unable to open manga archive";
    requestUpdate();
    return;
  }

  progress = std::make_unique<MangaProgress>(archive->cachePath());
  uint32_t savedPage = 0;
  pageIndex = (progress->load(savedPage) && savedPage < archive->pageCount()) ? savedPage : 0;
  ready = true;
  error = false;
  showControls = false;
  requestUpdate();

  prefetchAdjacent();
}

void MangaReaderActivity::onExit() {
  saveProgress();
  currentBmpPath.clear();
  progress.reset();
  archive.reset();
  ready = false;
  Activity::onExit();
}

void MangaReaderActivity::saveProgress() {
  if (progress) progress->save(static_cast<uint32_t>(pageIndex));
}

void MangaReaderActivity::prefetchAdjacent() {
  if (!ready || !archive) return;
  const auto settings = X3Plus::MangaSettings::load(MANGA_SETTINGS_PATH);
  if (!settings.prefetchNext || pageIndex + 1 >= archive->pageCount()) return;

  std::string unused;
  archive->materializePage(pageIndex + 1, unused, renderer.getScreenWidth(), renderer.getScreenHeight());
}

void MangaReaderActivity::moveNext() {
  if (!ready || archive->pageCount() == 0 || pageIndex + 1 >= archive->pageCount()) return;
  ++pageIndex;
  showControls = false;
  saveProgress();
  prefetchAdjacent();
  requestUpdate();
}

void MangaReaderActivity::movePrevious() {
  if (!ready || archive->pageCount() == 0 || pageIndex == 0) return;
  --pageIndex;
  showControls = false;
  saveProgress();
  requestUpdate();
}

void MangaReaderActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    finish();
    return;
  }

  const auto settings = X3Plus::MangaSettings::load(MANGA_SETTINGS_PATH);
  if (mappedInput.wasReleased(MappedInputManager::Button::PageForward)) {
    settings.rtl ? movePrevious() : moveNext();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::PageBack)) {
    settings.rtl ? moveNext() : movePrevious();
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    showControls = !showControls;
    requestUpdate();
  }
}

bool MangaReaderActivity::renderCurrentPage() {
  if (!ready || !archive) return false;

  const int screenWidth = renderer.getScreenWidth();
  const int screenHeight = renderer.getScreenHeight();
  if (!archive->materializePage(pageIndex, currentBmpPath, screenWidth, screenHeight)) {
    error = true;
    errorMessage = "Unable to decode page";
    return false;
  }

  HalFile file;
  if (!Storage.openFileForRead("MANGA", currentBmpPath.c_str(), file)) {
    error = true;
    errorMessage = "Unable to read decoded page";
    return false;
  }

  Bitmap bitmap(file);
  if (bitmap.parseHeaders() != BmpReaderError::Ok) {
    file.close();
    error = true;
    errorMessage = "Invalid decoded image";
    return false;
  }

  const int imageWidth = bitmap.getWidth();
  const int imageHeight = bitmap.getHeight();
  renderer.clearScreen();

  int x = 0;
  int y = 0;
  int maxWidth = screenWidth;
  int maxHeight = screenHeight;
  if (imageWidth <= screenWidth && imageHeight <= screenHeight) {
    x = (screenWidth - imageWidth) / 2;
    y = (screenHeight - imageHeight) / 2;
    maxWidth = imageWidth;
    maxHeight = imageHeight;
  }

  renderer.drawBitmap(bitmap, x, y, maxWidth, maxHeight);
  file.close();

  if (showControls) {
    const std::string title = "Manga  " + std::to_string(pageIndex + 1) + "/" + std::to_string(archive->pageCount());
    renderer.fillRect(0, 0, screenWidth, 24, false);
    renderer.drawText(SMALL_FONT_ID, 8, 5, title.c_str());
  }
  return true;
}

void MangaReaderActivity::render(RenderLock&&) {
  if (error) {
    renderer.clearScreen();
    GUI.drawHeader(renderer, Rect{0, 0, renderer.getScreenWidth(), 40}, "Manga");
    renderer.drawCenteredText(UI_12_FONT_ID, renderer.getScreenHeight() / 2, errorMessage.c_str());
    const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
    GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
    renderer.displayBuffer();
    return;
  }

  if (!ready || !renderCurrentPage()) {
    renderer.clearScreen();
    renderer.drawCenteredText(UI_12_FONT_ID, renderer.getScreenHeight() / 2, "Loading...");
  }
  renderer.displayBuffer();
}
