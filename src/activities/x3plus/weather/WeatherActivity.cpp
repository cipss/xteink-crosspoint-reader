#include "WeatherActivity.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <HalStorage.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr char WEATHER_CONFIG[] = "/.crosspoint/x3plus/weather.cfg";
constexpr char OPEN_METEO_HOST[] = "https://api.open-meteo.com";
constexpr int REQUEST_TIMEOUT_MS = 8000;
}

bool WeatherActivity::loadConfig() {
  latitude = 0.0f;
  longitude = 0.0f;

  auto file = Storage.open(WEATHER_CONFIG);
  if (!file || file.isDirectory()) {
    if (file) file.close();
    configured = false;
    return false;
  }

  char buf[256] = {};
  const auto size = std::min<std::size_t>(file.size(), sizeof(buf) - 1);
  const auto read = file.read(buf, size);
  file.close();
  if (read == 0) {
    configured = false;
    return false;
  }
  buf[read] = '\0';

  char* end = nullptr;
  const char* latStart = std::strstr(buf, "latitude=");
  const char* lonStart = std::strstr(buf, "longitude=");
  if (!latStart || !lonStart) {
    configured = false;
    return false;
  }

  latitude = std::strtof(latStart + 9, &end);
  if (!end || end == latStart + 9) {
    configured = false;
    return false;
  }
  longitude = std::strtof(lonStart + 10, &end);
  if (!end || end == lonStart + 10) {
    configured = false;
    return false;
  }

  configured = latitude >= -90.0f && latitude <= 90.0f && longitude >= -180.0f && longitude <= 180.0f;
  return configured;
}

bool WeatherActivity::fetchWeather() {
  if (!configured || WiFi.status() != WL_CONNECTED) return false;

  char url[512];
  std::snprintf(
      url, sizeof(url),
      "%s/v1/forecast?latitude=%.5f&longitude=%.5f&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code&daily=weather_code,temperature_2m_max,temperature_2m_min&forecast_days=%d&timezone=auto",
      OPEN_METEO_HOST, latitude, longitude, FORECAST_DAYS);

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.setTimeout(REQUEST_TIMEOUT_MS);
  if (!http.begin(client, url)) return false;

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    return false;
  }

  const String payload = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, payload)) return false;
  const auto current = doc["current"];
  const auto daily = doc["daily"];
  if (current.isNull() || daily.isNull()) return false;

  weather.temperature = current["temperature_2m"] | 0.0f;
  weather.apparentTemperature = current["apparent_temperature"] | 0.0f;
  weather.humidity = current["relative_humidity_2m"] | 0;
  weather.weatherCode = current["weather_code"] | -1;
  weather.valid = weather.weatherCode >= 0;

  const auto dates = daily["time"].as<JsonArray>();
  const auto maxTemps = daily["temperature_2m_max"].as<JsonArray>();
  const auto minTemps = daily["temperature_2m_min"].as<JsonArray>();
  const auto codes = daily["weather_code"].as<JsonArray>();
  if (dates.isNull() || maxTemps.isNull() || minTemps.isNull() || codes.isNull()) return false;

  forecast = {};
  for (int i = 0; i < FORECAST_DAYS; ++i) {
    if (i >= static_cast<int>(dates.size()) || i >= static_cast<int>(maxTemps.size()) ||
        i >= static_cast<int>(minTemps.size()) || i >= static_cast<int>(codes.size())) {
      break;
    }
    forecast[i].date = dates[i].as<const char*>();
    forecast[i].maxTemperature = maxTemps[i] | 0.0f;
    forecast[i].minTemperature = minTemps[i] | 0.0f;
    forecast[i].weatherCode = codes[i] | -1;
  }

  return weather.valid;
}

const char* WeatherActivity::describeWeatherCode(int code) {
  switch (code) {
    case 0: return "Sereno";
    case 1: case 2: case 3: return "Nuvoloso";
    case 45: case 48: return "Nebbia";
    case 51: case 53: case 55: case 56: case 57: return "Pioviggine";
    case 61: case 63: case 65: case 66: case 67: return "Pioggia";
    case 71: case 73: case 75: case 77: return "Neve";
    case 80: case 81: case 82: return "Rovesci";
    case 85: case 86: return "Rovesci di neve";
    case 95: case 96: case 99: return "Temporale";
    default: return "Condizioni variabili";
  }
}

void WeatherActivity::onEnter() {
  Activity::onEnter();
  weather = {};
  forecast = {};
  statusMessage.clear();
  configured = loadConfig();
  loading = false;
  if (configured && WiFi.status() == WL_CONNECTED) fetchWeather();
  requestUpdate();
}

void WeatherActivity::loop() {
  if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
    onGoHome(HomeMenuItem::X3PLUS);
    return;
  }

  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (!configured) return;
    loading = true;
    statusMessage.clear();
    requestUpdate(true);
    weather.valid = fetchWeather();
    loading = false;
    if (!weather.valid) statusMessage = "Aggiornamento non riuscito";
    requestUpdate();
  }
}

void WeatherActivity::render(RenderLock&&) {
  const auto& metrics = UITheme::getInstance().getMetrics();
  const int width = renderer.getScreenWidth();
  const int height = renderer.getScreenHeight();

  renderer.clearScreen();
  GUI.drawHeader(renderer, Rect{0, metrics.topPadding, width, metrics.headerHeight}, "Weather");

  if (!configured) {
    renderer.drawCenteredText(UI_12_FONT_ID, height / 2 - 20, "Weather non configurato");
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2 + 15, "Crea /.crosspoint/x3plus/weather.cfg");
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2 + 38, "latitude=...   longitude=...");
  } else if (loading) {
    renderer.drawCenteredText(UI_12_FONT_ID, height / 2, "Aggiornamento...");
  } else if (!weather.valid) {
    renderer.drawCenteredText(UI_12_FONT_ID, height / 2 - 20,
                              WiFi.status() == WL_CONNECTED ? "Nessun dato meteo" : "Connetti il Wi-Fi");
    renderer.drawCenteredText(UI_10_FONT_ID, height / 2 + 18, "Premi OK per aggiornare");
    if (!statusMessage.empty()) renderer.drawCenteredText(UI_10_FONT_ID, height / 2 + 42, statusMessage.c_str());
  } else {
    const std::string temp = std::to_string(static_cast<int>(weather.temperature + 0.5f)) + " C";
    const std::string feels = "Percepita " + std::to_string(static_cast<int>(weather.apparentTemperature + 0.5f)) + " C";
    const std::string humidity = "Umidita " + std::to_string(weather.humidity) + "%";
    renderer.drawCenteredText(UI_12_FONT_ID, 76, temp.c_str());
    renderer.drawCenteredText(UI_12_FONT_ID, 108, describeWeatherCode(weather.weatherCode));
    renderer.drawCenteredText(UI_10_FONT_ID, 136, feels.c_str());
    renderer.drawCenteredText(UI_10_FONT_ID, 158, humidity.c_str());

    int y = 190;
    for (int i = 0; i < FORECAST_DAYS; ++i) {
      if (forecast[i].date.empty()) continue;
      const std::string day = forecast[i].date + "  " + describeWeatherCode(forecast[i].weatherCode);
      const std::string temps = std::to_string(static_cast<int>(forecast[i].minTemperature + 0.5f)) + " / " +
                                 std::to_string(static_cast<int>(forecast[i].maxTemperature + 0.5f)) + " C";
      renderer.drawCenteredText(UI_10_FONT_ID, y, day.c_str());
      renderer.drawCenteredText(UI_10_FONT_ID, y + 20, temps.c_str());
      y += 48;
    }
  }

  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_SELECT), "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  renderer.displayBuffer();
}
