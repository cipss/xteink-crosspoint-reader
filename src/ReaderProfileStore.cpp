#include "ReaderProfileStore.h"

#include <ArduinoJson.h>
#include <HalStorage.h>
#include <Logging.h>

#include <algorithm>
#include <cstring>

#include "CrossPointSettings.h"

namespace {
constexpr char PROFILE_STORE_PATH[] = "/.crosspoint/reader_profiles.json";
constexpr uint8_t PROFILE_STORE_VERSION = 1;

std::string normalizeProfileId(const std::string& raw) {
  if (raw == "ROMANZO" || raw == "STUDIO" || raw == "FOCUS" || raw == "PDF" || raw == "PERSONALIZZATO") {
    return raw;
  }
  return "PERSONALIZZATO";
}
}  // namespace

ReaderProfileStore ReaderProfileStore::instance;

bool ReaderProfileStore::saveToFile() const {
  Storage.mkdir("/.crosspoint");
  JsonDocument doc;
  doc["profileVersion"] = PROFILE_STORE_VERSION;
  doc["activeProfileId"] = activeProfileId;
  JsonArray arr = doc["profiles"].to<JsonArray>();
  for (const auto& profile : profiles) {
    JsonObject obj = arr.add<JsonObject>();
    obj["id"] = profile.id;
    obj["isBuiltIn"] = profile.isBuiltIn;
    obj["isProtected"] = profile.isProtected;
    obj["fontFamily"] = profile.fontFamily;
    obj["sdFontFamilyName"] = profile.sdFontFamilyName;
    obj["fontSize"] = profile.fontSize;
    obj["lineSpacing"] = profile.lineSpacing;
    obj["paragraphAlignment"] = profile.paragraphAlignment;
    obj["extraParagraphSpacing"] = profile.extraParagraphSpacing;
    obj["screenMargin"] = profile.screenMargin;
    obj["embeddedStyle"] = profile.embeddedStyle;
    obj["focusReadingEnabled"] = profile.focusReadingEnabled;
    obj["hyphenationEnabled"] = profile.hyphenationEnabled;
    obj["textAntiAliasing"] = profile.textAntiAliasing;
    obj["imageRendering"] = profile.imageRendering;
    obj["orientation"] = profile.orientation;
    obj["refreshFrequency"] = profile.refreshFrequency;
  }
  String output;
  serializeJson(doc, output);
  return Storage.writeFile(PROFILE_STORE_PATH, output);
}

bool ReaderProfileStore::loadFromFile() {
  if (!Storage.exists(PROFILE_STORE_PATH)) {
    return false;
  }

  String json = Storage.readFile(PROFILE_STORE_PATH);
  if (json.isEmpty()) {
    LOG_ERR("RPS", "Profile store file is empty; falling back to defaults");
    return false;
  }

  JsonDocument doc;
  const auto error = deserializeJson(doc, json.c_str());
  if (error) {
    LOG_ERR("RPS", "JSON parse error: %s", error.c_str());
    return false;
  }

  const auto version = doc["profileVersion"] | 1;
  if (version != PROFILE_STORE_VERSION) {
    LOG_ERR("RPS", "Unknown profile version: %u", static_cast<unsigned>(version));
    return false;
  }

  profiles.clear();
  JsonArray arr = doc["profiles"].as<JsonArray>();
  for (JsonObject obj : arr) {
    String output;
    serializeJson(obj, output);
    const auto loaded = ReaderProfile::deserializeFromJsonString(output.c_str());
    if (!loaded.has_value()) {
      continue;
    }
    profiles.push_back(*loaded);
  }

  if (profiles.empty()) {
    LOG_ERR("RPS", "No valid reader profiles; falling back to defaults");
    return false;
  }

  activeProfileId = normalizeProfileId(doc["activeProfileId"] | std::string("PERSONALIZZATO"));
  if (findProfileById(activeProfileId) == nullptr) {
    activeProfileId = profiles.front().id;
  }
  return true;
}

bool ReaderProfileStore::ensureDefaultProfiles(const CrossPointSettings& settings) {
  if (Storage.exists(PROFILE_STORE_PATH)) {
    return false;
  }
  profiles = ReaderProfile::buildDefaultProfiles(settings);
  activeProfileId = "PERSONALIZZATO";
  return saveToFile();
}

bool ReaderProfileStore::setActiveProfile(const std::string& id) {
  const std::string normalizedId = normalizeProfileId(id);
  const ReaderProfile* profile = findProfileById(normalizedId);
  if (!profile) {
    return false;
  }

  activeProfileId = normalizedId;
  return saveToFile();
}

const ReaderProfile* ReaderProfileStore::getActiveProfile() const {
  return findProfileById(activeProfileId);
}

const ReaderProfile* ReaderProfileStore::findProfileById(const std::string& id) const {
  const std::string normalizedId = normalizeProfileId(id);
  auto it = std::find_if(profiles.begin(), profiles.end(), [&](const ReaderProfile& profile) {
    return profile.id == normalizedId;
  });
  return it == profiles.end() ? nullptr : &(*it);
}

void ReaderProfileStore::clear() {
  profiles.clear();
  activeProfileId = "PERSONALIZZATO";
}
