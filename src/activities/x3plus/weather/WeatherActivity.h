#pragma once

#include <string>

#include "activities/Activity.h"

class WeatherActivity final : public Activity {
 private:
  struct WeatherData {
    float temperature = 0;
    float apparentTemperature = 0;
    int humidity = 0;
    int weatherCode = -1;
    bool valid = false;
  };

  WeatherData weather;
  float latitude = 0.0f;
  float longitude = 0.0f;
  bool configured = false;
  bool loading = false;
  std::string statusMessage;

  bool loadConfig();
  bool fetchWeather();
  static const char* describeWeatherCode(int code);

 public:
  explicit WeatherActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Weather", renderer, mappedInput) {}

  void onEnter() override;
  void loop() override;
  void render(RenderLock&&) override;
};
