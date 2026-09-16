add_executable(calendar_ics_test
  CalendarIcsTest.cpp
  ${REPO_ROOT}/src/activities/x3plus/calendar/CalendarIcs.cpp
)

target_include_directories(calendar_ics_test PRIVATE
  ${REPO_ROOT}/src
  ${REPO_ROOT}/src/activities/x3plus
)
target_link_libraries(calendar_ics_test PRIVATE GTest::gtest_main crosspoint_test_common)

gtest_discover_tests(calendar_ics_test)
