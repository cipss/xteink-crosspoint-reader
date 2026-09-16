#include "X3PlusLauncherActivity.h"

#include "components/UITheme.h"
#include "x3plus/X3PlusAppRegistry.h"

namespace {

X3Plus::UIIcon iconForApp(X3Plus::AppId id) {
  switch (id) {
    case X3Plus::AppId::Manga:
      return Book;
    case X3Plus::AppId::Notes:
      return Text;
    case X3Plus::AppId::Weather:
      return Wifi;
    case X3Plus::AppId::Calendar:
      return Library;
    case X3Plus::AppId::Rss:
      return Text;
    case X3Plus::AppId::Browser:
      return Text;
    case X3Plus::AppId::Games:
      return Book;
    case X3Plus::AppId::Settings:
      return Settings;
  }
  return None;
}

}  // namespace

void X3PlusLauncherActivity::onEnter() {
  Activity::onEnter();
  selectorIndex = 0;
  showPlaceholder = false;
  placeholderMessage.clear();
  requestUpdate();
}

void X3PlusLauncherActivity::showPlaceholderFor(const char* appName) {
  placeholderMessage = std::string(appName) + " - coming soon";
  showPlaceholder = true;
  requestUpdate();
}

void X3PlusLauncherActivity::selectApp() {
  const auto* appList = X3Plus::apps();
  const auto count = X3Plus::appCount();

  if (selectorIndex < 0 || static_cast<std::size_t>(selectorIndex) >= count) return;

  const auto& app = appList[selectorIndex];
  if (app.status == X3Plus::AppStatus::Available) {
    showPlaceholderFor(app.title);
    return;
  }

  showPlaceholderFor(app.title);
}

void X3PlusLauncherActivity::loop() {
  const int appCount = static_cast<int>(X3Plus::appCount());

  buttonNavigator.onNext([this, appCount] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, appCount);
    showPlaceholder = false;
    requestUpdate();
  });

  buttonNavigator.onPrevious([this, appCount] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, appCount);
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

  const auto* appList = X3Plus::apps();
  const auto count = X3Plus::appCount();

  const Rect listRect{
      0,
      metrics.topPadding + metrics.headerHeight,
      pageWidth,
      pageHeight - (metrics.topPadding + metrics.headerHeight + metrics.buttonHintsHeight),
  };

  GUI.drawList(
      renderer,
      listRect,
      static_cast<int>(count),
      selectorIndex,
      [appList](int index) { return std::string(appList[index].title); },
      [appList](int index) { return std::string(appList[index].description); },
      [appList](int index) { return iconForApp(appList[index].id); });

  if (showPlaceholder) {
    GUI.drawPopup(renderer, placeholderMessage.c_str());
  }

  const auto labels = mappedInput.mapLabels("", tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
