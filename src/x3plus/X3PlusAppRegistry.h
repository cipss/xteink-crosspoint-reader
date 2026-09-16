#pragma once

#include <cstddef>

namespace X3Plus {

enum class AppId : uint8_t {
  Manga = 0,
  Notes,
  Weather,
  Calendar,
  Rss,
  Browser,
  Games,
  Settings,
};

enum class AppStatus : uint8_t {
  Planned = 0,
  Available,
};

struct AppInfo {
  AppId id;
  const char* title;
  const char* description;
  AppStatus status;
  bool requiresNetwork;
};

const AppInfo* apps();
std::size_t appCount();
const AppInfo* findApp(AppId id);

}  // namespace X3Plus
