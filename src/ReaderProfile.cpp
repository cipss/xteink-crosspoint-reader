#include "ReaderProfile.h"

#include <ArduinoJson.h>
#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>
#include <cstring>

#include "CrossPointSettings.h"

namespace {
constexpr uint8_t kProfileVersion = 1;
constexpr uint8_t kDefaultBuiltInProfileCount = 4;

bool isSupportedBuiltInId(const std::string& id) {
  return id == "ROMANZO" || id == "STUDIO" || id == "FOCUS" || id == "PDF" || id == "PERSONALIZZATO";
}
}  // namespace

ReaderProfile ReaderProfile::createRomanzo() {
  ReaderProfile profile;
  profile.id = "ROMANZO";
  profile.isBuiltIn = true;
  profile.isProtected = true;
  profile.fontFamily = CrossPointSettings::NOTOSERIF;
  profile.sdFontFamilyName[0] = '\0';
  profile.fontSize = CrossPointSettings::MEDIUM;
  profile.lineSpacing = CrossPointSettings::NORMAL;
  profile.paragraphAlignment = CrossPointSettings::JUSTIFIED;
  profile.extraParagraphSpacing = 1;
  profile.screenMargin = 10;
  profile.embeddedStyle = 1;
  profile.focusReadingEnabled = 0;
  profile.hyphenationEnabled = 1;
  profile.textAntiAliasing = 1;
  profile.imageRendering = CrossPointSettings::IMAGES_DISPLAY;
  profile.orientation = CrossPointSettings::PORTRAIT;
  profile.refreshFrequency = CrossPointSettings::REFRESH_15;
  return profile;
}

ReaderProfile ReaderProfile::createStudio() {
  ReaderProfile profile;
  profile.id = "STUDIO";
  profile.isBuiltIn = true;
  profile.isProtected = true;
  profile.fontFamily = CrossPointSettings::NOTOSANS;
  profile.sdFontFamilyName[0] = '\0';
  profile.fontSize = CrossPointSettings::LARGE;
  profile.lineSpacing = CrossPointSettings::WIDE;
  profile.paragraphAlignment = CrossPointSettings::LEFT_ALIGN;
  profile.extraParagraphSpacing = 1;
  profile.screenMargin = 12;
  profile.embeddedStyle = 1;
  profile.focusReadingEnabled = 0;
  profile.hyphenationEnabled = 1;
  profile.textAntiAliasing = 1;
  profile.imageRendering = CrossPointSettings::IMAGES_DISPLAY;
  profile.orientation = CrossPointSettings::PORTRAIT;
  profile.refreshFrequency = CrossPointSettings::REFRESH_10;
  return profile;
}

ReaderProfile ReaderProfile::createFocus() {
  ReaderProfile profile;
  profile.id = "FOCUS";
  profile.isBuiltIn = true;
  profile.isProtected = true;
  profile.fontFamily = CrossPointSettings::NOTOSANS;
  profile.sdFontFamilyName[0] = '\0';
  profile.fontSize = CrossPointSettings::MEDIUM;
  profile.lineSpacing = CrossPointSettings::NORMAL;
  profile.paragraphAlignment = CrossPointSettings::LEFT_ALIGN;
  profile.extraParagraphSpacing = 0;
  profile.screenMargin = 8;
  profile.embeddedStyle = 1;
  profile.focusReadingEnabled = 1;
  profile.hyphenationEnabled = 1;
  profile.textAntiAliasing = 1;
  profile.imageRendering = CrossPointSettings::IMAGES_DISPLAY;
  profile.orientation = CrossPointSettings::PORTRAIT;
  profile.refreshFrequency = CrossPointSettings::REFRESH_15;
  return profile;
}

ReaderProfile ReaderProfile::createPdf() {
  ReaderProfile profile;
  profile.id = "PDF";
  profile.isBuiltIn = true;
  profile.isProtected = true;
  profile.fontFamily = CrossPointSettings::NOTOSERIF;
  profile.sdFontFamilyName[0] = '\0';
  profile.fontSize = CrossPointSettings::MEDIUM;
  profile.lineSpacing = CrossPointSettings::NORMAL;
  profile.paragraphAlignment = CrossPointSettings::LEFT_ALIGN;
  profile.extraParagraphSpacing = 0;
  profile.screenMargin = 8;
  profile.embeddedStyle = 0;
  profile.focusReadingEnabled = 0;
  profile.hyphenationEnabled = 0;
  profile.textAntiAliasing = 1;
  profile.imageRendering = CrossPointSettings::IMAGES_DISPLAY;
  profile.orientation = CrossPointSettings::PORTRAIT;
  profile.refreshFrequency = CrossPointSettings::REFRESH_10;
  return profile;
}

ReaderProfile ReaderProfile::createPersonalizzatoFromSettings(const CrossPointSettings& settings) {
  ReaderProfile profile;
  profile.id = "PERSONALIZZATO";
  profile.isBuiltIn = false;
  profile.isProtected = true;
  profile.fontFamily = settings.fontFamily;
  profile.sdFontFamilyName[0] = '\0';
  if (settings.sdFontFamilyName[0] != '\0') {
    strncpy(profile.sdFontFamilyName, settings.sdFontFamilyName, sizeof(profile.sdFontFamilyName) - 1);
    profile.sdFontFamilyName[sizeof(profile.sdFontFamilyName) - 1] = '\0';
  }
  profile.fontSize = settings.fontSize;
  profile.lineSpacing = settings.lineSpacing;
  profile.paragraphAlignment = settings.paragraphAlignment;
  profile.extraParagraphSpacing = settings.extraParagraphSpacing;
  profile.screenMargin = settings.screenMargin;
  profile.embeddedStyle = settings.embeddedStyle;
  profile.focusReadingEnabled = settings.focusReadingEnabled;
  profile.hyphenationEnabled = settings.hyphenationEnabled;
  profile.textAntiAliasing = settings.textAntiAliasing;
  profile.imageRendering = settings.imageRendering;
  profile.orientation = settings.orientation;
  profile.refreshFrequency = settings.refreshFrequency;
  return profile;
}

std::vector<ReaderProfile> ReaderProfile::buildDefaultProfiles(const CrossPointSettings& settings) {
  std::vector<ReaderProfile> profiles;
  profiles.push_back(createRomanzo());
  profiles.push_back(createStudio());
  profiles.push_back(createFocus());
  profiles.push_back(createPdf());
  profiles.push_back(createPersonalizzatoFromSettings(settings));
  return profiles;
}

std::string ReaderProfile::serializeToJsonString() const {
  JsonDocument doc;
  doc["profileVersion"] = kProfileVersion;
  doc["id"] = id;
  doc["isBuiltIn"] = isBuiltIn;
  doc["isProtected"] = isProtected;
  doc["fontFamily"] = fontFamily;
  doc["sdFontFamilyName"] = sdFontFamilyName;
  doc["fontSize"] = fontSize;
  doc["lineSpacing"] = lineSpacing;
  doc["paragraphAlignment"] = paragraphAlignment;
  doc["extraParagraphSpacing"] = extraParagraphSpacing;
  doc["screenMargin"] = screenMargin;
  doc["embeddedStyle"] = embeddedStyle;
  doc["focusReadingEnabled"] = focusReadingEnabled;
  doc["hyphenationEnabled"] = hyphenationEnabled;
  doc["textAntiAliasing"] = textAntiAliasing;
  doc["imageRendering"] = imageRendering;
  doc["orientation"] = orientation;
  doc["refreshFrequency"] = refreshFrequency;

  String output;
  serializeJson(doc, output);
  return std::string(output.c_str());
}

std::optional<ReaderProfile> ReaderProfile::deserializeFromJsonString(const std::string& json) {
  JsonDocument doc;
  const auto error = deserializeJson(doc, json.c_str());
  if (error) {
    LOG_ERR("RPR", "JSON parse error: %s", error.c_str());
    return std::nullopt;
  }

  const auto version = doc["profileVersion"] | 1;
  if (version != kProfileVersion) {
    LOG_ERR("RPR", "Unknown profile version: %u", static_cast<unsigned>(version));
    return std::nullopt;
  }

  ReaderProfile profile;
  profile.id = doc["id"] | std::string("PERSONALIZZATO");
  if (!isSupportedBuiltInId(profile.id) && profile.id != "PERSONALIZZATO") {
    profile.id = "PERSONALIZZATO";
  }
  profile.isBuiltIn = doc["isBuiltIn"] | false;
  profile.isProtected = doc["isProtected"] | (profile.id != "PERSONALIZZATO");
  profile.fontFamily = doc["fontFamily"] | static_cast<uint8_t>(CrossPointSettings::NOTOSERIF);
  const char* sdName = doc["sdFontFamilyName"] | "";
  strncpy(profile.sdFontFamilyName, sdName, sizeof(profile.sdFontFamilyName) - 1);
  profile.sdFontFamilyName[sizeof(profile.sdFontFamilyName) - 1] = '\0';
  profile.fontSize = doc["fontSize"] | static_cast<uint8_t>(CrossPointSettings::MEDIUM);
  profile.lineSpacing = doc["lineSpacing"] | static_cast<uint8_t>(CrossPointSettings::NORMAL);
  profile.paragraphAlignment = doc["paragraphAlignment"] | static_cast<uint8_t>(CrossPointSettings::JUSTIFIED);
  profile.extraParagraphSpacing = doc["extraParagraphSpacing"] | static_cast<uint8_t>(1);
  profile.screenMargin = doc["screenMargin"] | static_cast<uint8_t>(5);
  profile.embeddedStyle = doc["embeddedStyle"] | static_cast<uint8_t>(1);
  profile.focusReadingEnabled = doc["focusReadingEnabled"] | static_cast<uint8_t>(0);
  profile.hyphenationEnabled = doc["hyphenationEnabled"] | static_cast<uint8_t>(0);
  profile.textAntiAliasing = doc["textAntiAliasing"] | static_cast<uint8_t>(1);
  profile.imageRendering = doc["imageRendering"] | static_cast<uint8_t>(CrossPointSettings::IMAGES_DISPLAY);
  profile.orientation = doc["orientation"] | static_cast<uint8_t>(CrossPointSettings::PORTRAIT);
  profile.refreshFrequency = doc["refreshFrequency"] | static_cast<uint8_t>(CrossPointSettings::REFRESH_15);

  return profile;
}

void ReaderProfile::applyToSettings(CrossPointSettings& settings) const {
  settings.fontFamily = fontFamily;
  settings.sdFontFamilyName[0] = '\0';
  if (sdFontFamilyName[0] != '\0') {
    strncpy(settings.sdFontFamilyName, sdFontFamilyName, sizeof(settings.sdFontFamilyName) - 1);
    settings.sdFontFamilyName[sizeof(settings.sdFontFamilyName) - 1] = '\0';
  }
  settings.fontSize = fontSize;
  settings.lineSpacing = lineSpacing;
  settings.paragraphAlignment = paragraphAlignment;
  settings.extraParagraphSpacing = extraParagraphSpacing;
  settings.screenMargin = screenMargin;
  settings.embeddedStyle = embeddedStyle;
  settings.focusReadingEnabled = focusReadingEnabled;
  settings.hyphenationEnabled = hyphenationEnabled;
  settings.textAntiAliasing = textAntiAliasing;
  settings.imageRendering = imageRendering;
  settings.orientation = orientation;
  settings.refreshFrequency = refreshFrequency;
}
