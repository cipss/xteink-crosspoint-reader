#pragma once

#include <I18n.h>

#include <string>
#include <vector>

#include "ReaderProfile.h"
#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class ReaderProfileSelectionActivity final : public Activity {
 public:
  explicit ReaderProfileSelectionActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("ReaderProfileSelection", renderer, mappedInput) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;

 private:
  void applySelectedProfile();
  void refreshProfiles();
  void onBack() { finish(); }

  ButtonNavigator buttonNavigator;
  int selectedIndex = 0;
  std::vector<ReaderProfile> profiles;
};
