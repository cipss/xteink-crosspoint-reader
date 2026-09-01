#include "ReaderProfileSelectionActivity.h"

#include <GfxRenderer.h>
#include <Logging.h>

#include <algorithm>

#include "CrossPointSettings.h"
#include "I18nKeys.h"
#include "MappedInputManager.h"
#include "ReaderProfileStore.h"
#include "components/UITheme.h"

namespace {
std::string profileDisplayName(const ReaderProfile& profile) {
  if (profile.id == "ROMANZO") return I18N.get(StrId::STR_ROMANZO);
  if (profile.id == "STUDIO") return I18N.get(StrId::STR_STUDIO);
  if (profile.id == "FOCUS") return I18N.get(StrId::STR_FOCUS);
  if (profile.id == "PDF") return I18N.get(StrId::STR_PDF);
  return I18N.get(StrId::STR_PERSONALIZZATO);
}
}  // namespace

void ReaderProfileSelectionActivity::onEnter() {
  Activity::onEnter();
  refreshProfiles();
  const auto activeId = READER_PROFILE_STORE.getActiveProfileId();
  auto it = std::find_if(profiles.begin(), profiles.end(), [&](const ReaderProfile& profile) { return profile.id == activeId; });
  selectedIndex = (it != profiles.end()) ? static_cast<int>(std::distance(profiles.begin(), it)) : 0;
  requestUpdate();
}

void ReaderProfileSelectionActivity::onExit() { Activity::onExit(); }

void ReaderProfileSelectionActivity::loop() {
  if (mappedInput.wasPressed(MappedInputManager::Button::Back)) {
    onBack();
    return;
  }

  if (mappedInput.wasPressed(MappedInputManager::Button::Confirm)) {
    applySelectedProfile();
    return;
  }

  const int pageItems = UITheme::getNumberOfItemsPerPage(renderer, true, false, true, false);

  buttonNavigator.onNextRelease([this] {
    selectedIndex = ButtonNavigator::nextIndex(selectedIndex, static_cast<int>(profiles.size()));
    requestUpdate();
  });

  buttonNavigator.onPreviousRelease([this] {
    selectedIndex = ButtonNavigator::previousIndex(selectedIndex, static_cast<int>(profiles.size()));
    requestUpdate();
  });

  buttonNavigator.onNextContinuous([this, pageItems] {
    selectedIndex = ButtonNavigator::nextPageIndex(selectedIndex, static_cast<int>(profiles.size()), pageItems);
    requestUpdate();
  });

  buttonNavigator.onPreviousContinuous([this, pageItems] {
    selectedIndex = ButtonNavigator::previousPageIndex(selectedIndex, static_cast<int>(profiles.size()), pageItems);
    requestUpdate();
  });
}

void ReaderProfileSelectionActivity::applySelectedProfile() {
  if (selectedIndex < 0 || selectedIndex >= static_cast<int>(profiles.size())) {
    return;
  }

  const auto& profile = profiles[selectedIndex];
  if (!READER_PROFILE_STORE.setActiveProfile(profile.id)) {
    LOG_ERR("RPS", "Failed to set active profile: %s", profile.id.c_str());
    return;
  }

  const ReaderProfile* active = READER_PROFILE_STORE.getActiveProfile();
  if (!active) {
    return;
  }

  const auto current = ReaderProfile::createPersonalizzatoFromSettings(SETTINGS);
  if (profile.id == "PERSONALIZZATO") {
    const auto personalizzato = current;
    personalizzato.applyToSettings(SETTINGS);
  } else {
    active->applyToSettings(SETTINGS);
  }
  SETTINGS.saveToFile();
  finish();
}

void ReaderProfileSelectionActivity::refreshProfiles() {
  profiles.clear();
  if (READER_PROFILE_STORE.getProfiles().empty()) {
    std::vector<ReaderProfile> defaults = ReaderProfile::buildDefaultProfiles(SETTINGS);
    READER_PROFILE_STORE.setProfiles(defaults);
    READER_PROFILE_STORE.setActiveProfile("PERSONALIZZATO");
    profiles = defaults;
  } else {
    profiles = READER_PROFILE_STORE.getProfiles();
  }
}

void ReaderProfileSelectionActivity::render(RenderLock&&) {
  renderer.clearScreen();

  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();

  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, pageWidth, metrics.headerHeight}, tr(STR_READING_PROFILES));

  const int contentTop = metrics.topPadding + metrics.headerHeight + metrics.verticalSpacing;
  const int contentHeight = pageHeight - contentTop - metrics.buttonHintsHeight - metrics.verticalSpacing;

  const auto activeId = READER_PROFILE_STORE.getActiveProfileId();
  GUI.drawList(renderer, Rect{0, contentTop, pageWidth, contentHeight}, static_cast<int>(profiles.size()), selectedIndex,
               [this](int index) { return profileDisplayName(profiles[index]); }, nullptr, nullptr,
               [this, activeId](int index) { return profiles[index].id == activeId ? tr(STR_ACTIVE) : ""; }, true);

  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);

  renderer.displayBuffer();
}
