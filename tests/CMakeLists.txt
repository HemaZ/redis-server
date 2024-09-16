enable_testing()

add_executable(
  parsing_test
  parser_test.cpp
)
target_link_libraries(
  parsing_test
  gtest gtest_main gmock
)

# add_executable(
#   rdb_test
#   rdb_test.cpp
# )
# target_link_libraries(
#   rdb_test
#   gtest gtest_main gmock
# )


include(GoogleTest)
gtest_discover_tests(parsing_test)
# gtest_discover_tests(rdb_test)