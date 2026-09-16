#pragma once

#include <string>
#include <vector>

namespace X3Plus {

struct CalendarEvent {
  std::string uid;
  std::string summary;
  std::string start;
  std::string end;
};

class CalendarIcs final {
 public:
  static bool parse(const std::string& data, std::vector<CalendarEvent>& events);
  static std::string formatDate(const std::string& value);

 private:
  static void unfold(const std::string& input, std::string& output);
};

}  // namespace X3Plus
