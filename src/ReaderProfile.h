#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class CrossPointSettings;

class ReaderProfile {
 public:
  std::string id = "PERSONALIZZATO";
  bool isBuiltIn = false;
  bool isProtected = false;

  uint8_t fontFamily = 0;
  char sdFontFamilyName[32] = "";
  uint8_t fontSize = 1;
  uint8_t lineSpacing = 1;
  uint8_t paragraphAlignment = 0;
  uint8_t extraParagraphSpacing = 1;
  uint8_t screenMargin = 5;
  uint8_t embeddedStyle = 1;
  uint8_t focusReadingEnabled = 0;
  uint8_t hyphenationEnabled = 0;
  uint8_t textAntiAliasing = 1;
  uint8_t imageRendering = 0;
  uint8_t orientation = 0;
  uint8_t refreshFrequency = 3;

  static ReaderProfile createRomanzo();
  static ReaderProfile createStudio();
  static ReaderProfile createFocus();
  static ReaderProfile createPdf();
  static ReaderProfile createPersonalizzatoFromSettings(const CrossPointSettings& settings);

  static std::vector<ReaderProfile> buildDefaultProfiles(const CrossPointSettings& settings);
  static std::optional<ReaderProfile> deserializeFromJsonString(const std::string& json);

  std::string serializeToJsonString() const;
  void applyToSettings(CrossPointSettings& settings) const;

  bool isBuiltInProfile() const { return isBuiltIn; }
  bool isProtectedProfile() const { return isProtected; }
};
