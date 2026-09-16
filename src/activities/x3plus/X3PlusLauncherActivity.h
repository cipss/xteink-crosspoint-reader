#pragma once

#include <string>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"

class X3PlusLauncherActivity final : public Activity {
 private:
  ButtonNavigator buttonNavigator;
  int selectorIndex = 0;
  bool showPlaceholder = false;
  std::string placeholderMessage;

  void selectApp();
  void showPlaceholderFor(const char* appName);

 public:
  explicit X3PlusLauncherActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("X3PlusLauncher", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
