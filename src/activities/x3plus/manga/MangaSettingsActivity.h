#pragma once

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"
#include "manga/MangaSettings.h"

class MangaSettingsActivity final : public Activity {
 private:
  ButtonNavigator buttonNavigator;
  X3Plus::MangaSettings settings;
  int selectorIndex = 0;
  static constexpr char SETTINGS_PATH[] = "/.crosspoint/x3plus/manga/settings.cfg";

  int itemCount() const { return 4; }
  std::string itemLabel(int index) const;
  void toggleSelected();
  void save();

 public:
  explicit MangaSettingsActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("MangaSettings", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
