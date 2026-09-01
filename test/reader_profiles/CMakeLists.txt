add_executable(ReaderProfilesTest
  ReaderProfilesTest.cpp
  ${REPO_ROOT}/src/ReaderProfile.cpp
  ${REPO_ROOT}/src/ReaderProfileStore.cpp
)

target_link_libraries(ReaderProfilesTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

target_include_directories(ReaderProfilesTest PRIVATE
  ${REPO_ROOT}/src
)

add_test(NAME ReaderProfilesTest COMMAND ReaderProfilesTest)
