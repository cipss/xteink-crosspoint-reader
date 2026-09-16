#include "MangaSettingsActivity.h"

#include "components/UITheme.h"

std::string MangaSettingsActivity::itemLabel(int index) const {
  switch (index) {
    case 0:
      return std::string("Reading direction: ") + (settings.rtl ? "RTL" : "LTR");
    case 1:
      switch (settings.fitMode) {
        case X3Plus::MangaFitMode::FitPage: return "Fit mode: Page";
        case X3Plus::MangaFitMode::FitWidth: return "Fit mode: Width";
        case X3Plus::MangaFitMode::FitHeight: return "Fit mode: Height";
        case X3Plus::MangaFitMode::Smart: return "Fit mode: Smart";
      }
      return "Fit mode: Smart";
    case 2:
      return std::string("Prefetch next page: ") + (settings.prefetchNext ? "On" : "Off");
    case 3:
      return std::string("Keep previous cache: ") + (settings.keepPreviousCache ? "On" : "Off");
    default:
      return {};
  }
}

void MangaSettingsActivity::save() {
  settings.save(SETTINGS_PATH);
  requestUpdate();
}

void MangaSettingsActivity::toggleSelected() {
  switch (selectorIndex) {
    case 0:
      settings.rtl = !settings.rtl;
      break;
    case 1:
      settings.fitMode = static_cast<X3Plus::MangaFitMode>((static_cast<int>(settings.fitMode) + 1) % 4);
      break;
    case 2:
      settings.prefetchNext = !settings.prefetchNext;
      break;
    case 3:
      settings.keepPreviousCache = !settings.keepPreviousCache;
      break;
    default:
      return;
  }
  save();
}

void MangaSettingsActivity::onEnter() {
  Activity::onEnter();
  settings = X3Plus::MangaSettings::load(SETTINGS_PATH);
  selectorIndex = 0;
  requestUpdate();
}

void MangaSettingsActivity::loop() {
  buttonNavigator.onNextRelease([this] {
    selectorIndex = ButtonNavigator::nextIndex(selectorIndex, itemCount());
    requestUpdate();
  });
  buttonNavigator.onPreviousRelease([this] {
    selectorIndex = ButtonNavigator::previousIndex(selectorIndex, itemCount());
    requestUpdate();
  });

  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome(HomeMenuItem::X3PLUS);
    return;
  }
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) toggleSelected();
}

void MangaSettingsActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, "Manga Settings");
  const Rect content{0, metrics.topPadding + metrics.headerHeight, width,
                     height - metrics.topPadding - metrics.headerHeight - metrics.buttonHintsHeight};
  GUI.drawList(renderer, content, itemCount(), selectorIndex,
               [this](int index) { return itemLabel(index); });

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
