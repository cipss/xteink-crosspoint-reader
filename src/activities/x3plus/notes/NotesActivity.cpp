#include "NotesActivity.h"

#include <HalStorage.h>
#include <Memory.h>

#include <algorithm>
#include <cctype>
#include <cstdio>

#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"
#include "reader/ReaderActivity.h"

namespace {
constexpr char NOTES_ROOT[] = "/Notes";
constexpr size_t MAX_TITLE_LENGTH = 48;
constexpr size_t MAX_BODY_LENGTH = 4096;
}

std::string NotesActivity::displayName(const std::string& path) {
  const auto slash = path.find_last_of('/');
  std::string name = slash == std::string::npos ? path : path.substr(slash + 1);
  if (name.size() >= 4 && name.compare(name.size() - 4, 4, ".txt") == 0) name.resize(name.size() - 4);
  return name;
}

std::string NotesActivity::makeSafeFileName(const std::string& title) {
  std::string safe;
  safe.reserve(title.size());
  for (unsigned char c : title) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == ' ') {
      safe.push_back(static_cast<char>(c));
    } else {
      safe.push_back('_');
    }
  }
  while (!safe.empty() && safe.back() == ' ') safe.pop_back();
  if (safe.empty()) safe = "Note";
  if (safe.size() > MAX_TITLE_LENGTH) safe.resize(MAX_TITLE_LENGTH);
  return safe;
}

void NotesActivity::loadNotes() {
  noteFiles.clear();
  auto root = Storage.open(NOTES_ROOT);
  if (!root) {
    Storage.mkdir(NOTES_ROOT);
    return;
  }
  if (!root.isDirectory()) {
    root.close();
    return;
  }

  root.rewindDirectory();
  char name[256];
  while (auto entry = root.openNextFile()) {
    entry.getName(name, sizeof(name));
    if (entry.isDirectory()) continue;
    const std::string filename(name);
    if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, ".txt") == 0) {
      noteFiles.push_back(std::string(NOTES_ROOT) + "/" + filename);
    }
  }
  root.close();
  std::sort(noteFiles.begin(), noteFiles.end());
  if (selectorIndex >= static_cast<int>(noteFiles.size())) {
    selectorIndex = noteFiles.empty() ? 0 : static_cast<int>(noteFiles.size()) - 1;
  }
}

bool NotesActivity::writeNote(const std::string& title, const std::string& body) {
  const std::string safe = makeSafeFileName(title);
  std::string path = std::string(NOTES_ROOT) + "/" + safe + ".txt";

  for (int suffix = 2; Storage.exists(path.c_str()) && suffix < 1000; ++suffix) {
    path = std::string(NOTES_ROOT) + "/" + safe + " (" + std::to_string(suffix) + ").txt";
  }
  if (Storage.exists(path.c_str())) return false;

  HalFile file;
  if (!Storage.openFileForWrite("NOTES", path.c_str(), file)) return false;
  const std::string content = title + "\n\n" + body + "\n";
  if (content.size() > MAX_BODY_LENGTH + MAX_TITLE_LENGTH + 2) {
    file.close();
    Storage.remove(path.c_str());
    return false;
  }
  const bool ok = file.write(content.data(), content.size()) == content.size();
  file.close();
  return ok;
}

void NotesActivity::createNote() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(renderer, mappedInput, "New note title", "", MAX_TITLE_LENGTH),
      [this](const ActivityResult& titleResult) {
        if (titleResult.isCancelled) return;
        const auto* title = std::get_if<KeyboardResult>(&titleResult.data);
        if (!title || title->text.empty()) return;

        startActivityForResult(
            std::make_unique<KeyboardEntryActivity>(renderer, mappedInput, "Note text", "", MAX_BODY_LENGTH),
            [this, titleText = title->text](const ActivityResult& bodyResult) {
              if (bodyResult.isCancelled) return;
              const auto* body = std::get_if<KeyboardResult>(&bodyResult.data);
              if (!body) return;
              writeNote(titleText, body->text);
              loadNotes();
              requestUpdate(true);
            });
      });
}

void NotesActivity::openSelected() {
  if (noteFiles.empty() || selectorIndex < 0 || selectorIndex >= static_cast<int>(noteFiles.size())) return;
  activityManager.pushActivity(std::make_unique<ReaderActivity>(renderer, mappedInput, noteFiles[selectorIndex]));
}

void NotesActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  loadNotes();
  requestUpdate();
}

void NotesActivity::onExit() {
  Activity::onExit();
  noteFiles.clear();
}

void NotesActivity::loop() {
  const int count = static_cast<int>(noteFiles.size()) + 1;
  buttonNavigator.onNextRelease([this, count] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, count);
    requestUpdate();
  });
  buttonNavigator.onPreviousRelease([this, count] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, count);
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome(HomeMenuItem::X3PLUS);
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (selectorIndex == static_cast<int>(noteFiles.size())) {
      createNote();
    } else {
      openSelected();
    }
  }
}

void NotesActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();
  const int totalItems = static_cast<int>(noteFiles.size()) + 1;

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, "Notes");

  const Rect content{0, metrics.topPadding + metrics.headerHeight, width,
                     height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight};
  GUI.drawList(
      renderer, content, totalItems, selectorIndex,
      [this](int index) {
        if (index == static_cast<int>(noteFiles.size())) return std::string("+ New note");
        return displayName(noteFiles[index]);
      },
      nullptr,
      [this](int index) { return index == static_cast<int>(noteFiles.size()) ? Text : File; });

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_OPEN), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
