#pragma once

#include <string>
#include <vector>

#include "activities/Activity.h"
#include "calendar/CalendarIcs.h"
#include "util/ButtonNavigator.h"

class CalendarActivity final : public Activity {
 private:
  ButtonNavigator buttonNavigator;
  std::vector<X3Plus::CalendarEvent> events;
  int selectorIndex = 0;
  std::string sourcePath;
  std::string syncUrl;
  bool syncing = false;
  std::string statusMessage;

  void loadEvents();
  void loadSyncUrl();
  bool fetchRemoteCalendar();
  void configureSyncUrl();
  static std::string eventLabel(const X3Plus::CalendarEvent& event);

 public:
  explicit CalendarActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Calendar", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
