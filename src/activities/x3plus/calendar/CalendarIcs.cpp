#include "CalendarIcs.h"

#include <algorithm>

namespace X3Plus {

void CalendarIcs::unfold(const std::string& input, std::string& output) {
  output.clear();
  std::size_t pos = 0;
  while (pos < input.size()) {
    const auto eol = input.find('\n', pos);
    const auto end = eol == std::string::npos ? input.size() : eol;
    std::string line = input.substr(pos, end - pos);
    if (!line.empty() && line.back() == '\r') line.pop_back();

    if (!line.empty() && (line[0] == ' ' || line[0] == '\t') && !output.empty()) {
      const auto previousEol = output.find_last_of('\n');
      if (previousEol != std::string::npos) {
        output.erase(previousEol + 1);
        output += line.substr(1);
      } else {
        output += line.substr(1);
      }
    } else {
      output += line;
      output.push_back('\n');
    }
    pos = eol == std::string::npos ? input.size() : eol + 1;
  }
}

bool CalendarIcs::parse(const std::string& data, std::vector<CalendarEvent>& events) {
  events.clear();
  std::string unfolded;
  unfold(data, unfolded);

  bool inEvent = false;
  CalendarEvent current;
  std::size_t pos = 0;
  while (pos < unfolded.size()) {
    const auto eol = unfolded.find('\n', pos);
    const auto end = eol == std::string::npos ? unfolded.size() : eol;
    const std::string line = unfolded.substr(pos, end - pos);
    pos = eol == std::string::npos ? unfolded.size() : eol + 1;

    if (line == "BEGIN:VEVENT") {
      inEvent = true;
      current = {};
      continue;
    }
    if (line == "END:VEVENT") {
      if (inEvent && (!current.summary.empty() || !current.start.empty())) events.push_back(current);
      inEvent = false;
      continue;
    }
    if (!inEvent) continue;

    const auto colon = line.find(':');
    if (colon == std::string::npos) continue;
    std::string key = line.substr(0, colon);
    const std::string value = line.substr(colon + 1);
    const auto semicolon = key.find(';');
    if (semicolon != std::string::npos) key.resize(semicolon);

    if (key == "UID") current.uid = value;
    else if (key == "SUMMARY") current.summary = value;
    else if (key == "DTSTART") current.start = value;
    else if (key == "DTEND") current.end = value;
  }

  std::sort(events.begin(), events.end(), [](const CalendarEvent& a, const CalendarEvent& b) {
    return a.start < b.start;
  });
  return !events.empty();
}

std::string CalendarIcs::formatDate(const std::string& value) {
  if (value.size() < 8) return value;
  std::string result = value.substr(6, 2) + "/" + value.substr(4, 2) + "/" + value.substr(0, 4);
  if (value.size() >= 13) result += " " + value.substr(9, 2) + ":" + value.substr(11, 2);
  return result;
}

}  // namespace X3Plus
