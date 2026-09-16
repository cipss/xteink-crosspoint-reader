#include "X3PlusAppRegistry.h"

namespace X3Plus {
namespace {

constexpr AppInfo kApps[] = {
    {AppId::Manga, "Manga", "Read image-based manga volumes", AppStatus::Planned, false},
    {AppId::Notes, "Notes", "Create and edit lightweight notes", AppStatus::Planned, false},
    {AppId::Weather, "Weather", "View current weather and forecast", AppStatus::Planned, true},
    {AppId::Calendar, "Calendar", "View upcoming calendar events", AppStatus::Planned, true},
    {AppId::Rss, "RSS / News", "Read RSS feeds and saved articles", AppStatus::Planned, true},
    {AppId::Browser, "Browser", "Browse lightweight text-oriented web pages", AppStatus::Planned, true},
    {AppId::Games, "Games", "Run lightweight e-ink games", AppStatus::Planned, false},
    {AppId::Settings, "X3+ Settings", "Configure X3+ services and apps", AppStatus::Available, false},
};

}  // namespace

const AppInfo* apps() { return kApps; }

std::size_t appCount() { return sizeof(kApps) / sizeof(kApps[0]); }

const AppInfo* findApp(AppId id) {
  for (const auto& app : kApps) {
    if (app.id == id) return &app;
  }
  return nullptr;
}

}  // namespace X3Plus
