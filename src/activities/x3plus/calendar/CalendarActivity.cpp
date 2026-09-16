#include "CalendarActivity.h"

#include <HalStorage.h>
#include <Memory.h>

#include <algorithm>

#include "components/UITheme.h"

namespace {
constexpr char CALENDAR_ROOT[] = "/Calendar";
constexpr size_t MAX_ICS_BYTES = 96 * 1024;
}

std::string CalendarActivity::eventLabel(const X3Plus::CalendarEvent& event) {
  return X3Plus::CalendarIcs::formatDate(event.start) + "  " +
         (event.summary.empty() ? std::string("(no title)") : event.summary);
}

void CalendarActivity::openCalendarFile() {
  if (events.empty() || selectorIndex < 0 || selectorIndex >= static_cast<int>(events.size())) return;
}

void CalendarActivity::loadEvents() {
  events.clear();
  sourcePath.clear();

  auto root = Storage.open(CALENDAR_ROOT);
  if (!root) {
    Storage.mkdir(CALENDAR_ROOT);
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
    if (filename.size() < 4 || filename.compare(filename.size() - 4, 4, ".ics") != 0) continue;

    const std::string path = std::string(CALENDAR_ROOT) + "/" + filename;
    HalFile file;
    if (!Storage.openFileForRead("CAL", path.c_str(), file)) continue;
    const auto fileSize = file.size();
    if (fileSize > MAX_ICS_BYTES) {
      file.close();
      continue;
    }

    std::unique_ptr<char[]> data = makeUniqueNoThrow<char[]>(fileSize + 1);
    if (!data) {
      file.close();
      continue;
    }
    const auto bytesRead = file.read(data.get(), fileSize);
    file.close();
    if (bytesRead != fileSize) continue;
    data[fileSize] = '\0';

    std::vector<X3Plus::CalendarEvent> parsed;
    if (X3Plus::CalendarIcs::parse(std::string(data.get(), fileSize), parsed)) {
      events.insert(events.end(), parsed.begin(), parsed.end());
      if (sourcePath.empty()) sourcePath = path;
    }
  }
  root.close();

  std::sort(events.begin(), events.end(), [](const auto& a, const auto& b) { return a.start < b.start; });
  if (selectorIndex >= static_cast<int>(events.size())) {
    selectorIndex = events.empty() ? 0 : static_cast<int>(events.size()) - 1;
  }
}

void CalendarActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  loadEvents();
  requestUpdate();
}

void CalendarActivity::loop() {
  const int count = static_cast<int>(events.size());
  buttonNavigator.onNextRelease([this, count] {
    if (count == 0) return;
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, count);
    requestUpdate();
  });
  buttonNavigator.onPreviousRelease([this, count] {
    if (count == 0) return;
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, count);
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome(HomeMenuItem::X3PLUS);
  }
}

void CalendarActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, "Calendar");

  const Rect content{0, metrics.topPadding + metrics.headerHeight, width,
                     height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight};
  if (events.empty()) {
    renderer.drawCenteredText(UI_12_FONT_ID, content.y + content.height / 2, "Nessun evento");
    renderer.drawCenteredText(UI_10_FONT_ID, content.y + content.height / 2 + 28,
                              "Metti un file .ics nella cartella /Calendar");
  } else {
    GUI.drawList(
        renderer, content, static_cast<int>(events.size()), selectorIndex,
        [this](int index) { return eventLabel(events[index]); },
        [this](int index) {
          return events[index].end.empty() ? std::string() : "fino a " + X3Plus::CalendarIcs::formatDate(events[index].end);
        });
  }

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), "", tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
