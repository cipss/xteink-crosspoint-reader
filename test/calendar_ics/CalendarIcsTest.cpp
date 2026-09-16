#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "x3plus/calendar/CalendarIcs.h"

TEST(CalendarIcs, ParsesEventsAndSortsByStart) {
  const std::string ics =
      "BEGIN:VCALENDAR\r\n"
      "VERSION:2.0\r\n"
      "BEGIN:VEVENT\r\n"
      "UID:b\r\n"
      "SUMMARY:Second event\r\n"
      "DTSTART:20260917T120000Z\r\n"
      "DTEND:20260917T130000Z\r\n"
      "END:VEVENT\r\n"
      "BEGIN:VEVENT\r\n"
      "UID:a\r\n"
      "SUMMARY:First event\r\n"
      "DTSTART;VALUE=DATE:20260916\r\n"
      "DTEND;VALUE=DATE:20260917\r\n"
      "END:VEVENT\r\n"
      "END:VCALENDAR\r\n";

  std::vector<X3Plus::CalendarEvent> events;
  ASSERT_TRUE(X3Plus::CalendarIcs::parse(ics, events));
  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(events[0].uid, "a");
  EXPECT_EQ(events[0].summary, "First event");
  EXPECT_EQ(events[1].uid, "b");
  EXPECT_EQ(X3Plus::CalendarIcs::formatDate("20260917T120000Z"), "17/09/2026 12:00");
}

TEST(CalendarIcs, UnfoldsContinuationLines) {
  const std::string ics =
      "BEGIN:VEVENT\n"
      "UID:1\n"
      "SUMMARY:Hello\n"
      " world\n"
      "DTSTART:20260916T090000\n"
      "END:VEVENT\n";
  std::vector<X3Plus::CalendarEvent> events;
  ASSERT_TRUE(X3Plus::CalendarIcs::parse(ics, events));
  ASSERT_EQ(events.size(), 1u);
  EXPECT_EQ(events[0].summary, "Hello world");
}
