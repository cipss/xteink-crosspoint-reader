#include "X3PlusLauncherActivity.h"

#include "components/UITheme.h"

const std::vector<std::string>& X3PlusLauncherActivity::appNames() {
  static const std::vector<std::string> apps = {
      "Manga",
      "Notes",
      "Weather",
      "Calendar",
      "RSS / News",
      "Browser",
      "Games",
      "X3+ Settings",
  };

  return apps;
}

void X3PlusLauncherActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  showPlaceholder = false;
  placeholderMessage.clear();
  requestUpdate();
}

void X3PlusLauncherActivity::showPlaceholderFor(const std::string& appName) {
  placeholderMessage = appName + " - coming soon";
  showPlaceholder = true;
  requestUpdate();
}

void X3PlusLauncherActivity::selectApp() {
  const auto& apps = appNames();
  if (selectorIndex >= 0 && selectorIndex < static_cast<int>(apps.size())) {
    showPlaceholderFor(apps[selectorIndex]);
  }
}

void X3PlusLauncherActivity::loop() {
  const auto& apps = appNames();

  buttonNavigator.onNext([this, &apps] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, static_cast<int>(apps.size()));
    showPlaceholder = false;
    requestUpdate();
  });

  buttonNavigator.onPrevious([this, &apps] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, static_cast<int>(apps.size()));
    showPlaceholder = false;
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome();
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    selectApp();
  }
}

void X3PlusLauncherActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int pageWidth = renderer.getScreenWidth();
  const int pageHeight = renderer.getScreenHeight();

  renderer.clearScreen();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, "CrossPoint X3+");

  const auto& apps = appNames();
  const Rect listRect{
      0,
      metrics.topPadding + metrics.headerHeight,
      pageWidth,
      pageHeight - (metrics.topPadding + metrics.headerHeight + metrics.buttonHintsHeight),
  };

  GUI.drawList(
      renderer,
      listRect,
      static_cast<int>(apps.size()),
      selectorIndex,
      [&apps](int index) { return apps[index]; },
      nullptr,
      [](int index) {
        switch (index) {
          case 0:
            return Book;
          case 1:
            return Text;
          case 2:
            return Wifi;
          case 3:
            return Library;
          case 4:
            return Text;
          case 5:
            return Text;
          case 6:
            return Book;
          case 7:
            return Settings;
          default:
            return None;
        }
      });

  if (showPlaceholder) {
    GUI.drawPopup(renderer, placeholderMessage.c_str());
  }

  const auto labels = mappedInput.mapLabels("", tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
