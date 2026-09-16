#include "CalendarActivity.h"

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HalStorage.h>
#include <Memory.h>

#include <algorithm>

#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr char CALENDAR_ROOT[] = "/Calendar";
constexpr char SYNC_URL_FILE[] = "/.crosspoint/x3plus/calendar.url";
constexpr char SYNC_CACHE_FILE[] = "/Calendar/remote.ics";
constexpr size_t MAX_ICS_BYTES = 96 * 1024;
}

std::string CalendarActivity::eventLabel(const X3Plus::CalendarEvent& event) {
  return X3Plus::CalendarIcs::formatDate(event.start) + "  " +
         (event.summary.empty() ? std::string("(no title)") : event.summary);
}

void CalendarActivity::loadSyncUrl() {
  syncUrl.clear();
  HalFile file;
  if (!Storage.openFileForRead("CAL", SYNC_URL_FILE, file)) return;
  const size_t size = std::min<std::size_t>(file.size(), 512);
  std::unique_ptr<char[]> buf = makeUniqueNoThrow<char[]>(size + 1);
  if (!buf) {
    file.close();
    return;
  }
  const auto read = file.read(buf.get(), size);
  file.close();
  if (read == 0) return;
  buf[read] = '\0';
  syncUrl.assign(buf.get(), read);
  while (!syncUrl.empty() && (syncUrl.back() == '\r' || syncUrl.back() == '\n' || syncUrl.back() == ' ')) syncUrl.pop_back();
}

bool CalendarActivity::fetchRemoteCalendar() {
  if (syncUrl.empty() || WiFi.status() != WL_CONNECTED) return false;
  HTTPClient http;
  http.setTimeout(10000);
  int code = 0;
  WiFiClientSecure client;
  client.setInsecure();
  if (!http.begin(client, syncUrl.c_str())) return false;
  code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    return false;
  }
  String payload = http.getString();
  http.end();
  if (payload.length() == 0 || payload.length() > MAX_ICS_BYTES) return false;

  HalFile file;
  if (!Storage.openFileForWrite("CAL", SYNC_CACHE_FILE, file)) return false;
  const bool ok = file.write(payload.c_str(), payload.length()) == payload.length();
  file.close();
  return ok;
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

void CalendarActivity::configureSyncUrl() {
  startActivityForResult(
      std::make_unique<KeyboardEntryActivity>(renderer, mappedInput, "Calendar ICS URL", syncUrl, 512, InputType::Url),
      [this](const ActivityResult& result) {
        if (result.isCancelled) return;
        const auto* keyboard = std::get_if<KeyboardResult>(&result.data);
        if (!keyboard) return;
        syncUrl = keyboard->text;
        HalFile file;
        if (Storage.openFileForWrite("CAL", SYNC_URL_FILE, file)) {
          file.write(syncUrl.data(), syncUrl.size());
          file.close();
        }
        if (fetchRemoteCalendar()) statusMessage = "Calendario sincronizzato";
        else statusMessage = "Sync non riuscito";
        loadEvents();
        requestUpdate(true);
      });
}

void CalendarActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  statusMessage.clear();
  loadSyncUrl();
  if (!syncUrl.empty()) fetchRemoteCalendar();
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
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    configureSyncUrl();
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
    renderer.drawCenteredText(UI_12_FONT_ID, content.y + content.height / 2 - 20, "Nessun evento");
    renderer.drawCenteredText(UI_10_FONT_ID, content.y + content.height / 2 + 12, "OK: configura URL ICS");
    renderer.drawCenteredText(UI_10_FONT_ID, content.y + content.height / 2 + 34,
                              "Apple Calendar / Google Calendar");
  } else {
    GUI.drawList(
        renderer, content, static_cast<int>(events.size()), selectorIndex,
        [this](int index) { return eventLabel(events[index]); },
        [this](int index) {
          return events[index].end.empty() ? std::string() : "fino a " + X3Plus::CalendarIcs::formatDate(events[index].end);
        });
  }

  if (!statusMessage.empty()) GUI.drawPopup(renderer, statusMessage.c_str());

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
