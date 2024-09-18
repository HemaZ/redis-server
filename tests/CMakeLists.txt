enable_testing()

add_executable(
  parsing_test
  parser_test.cpp
)
target_link_libraries(
  parsing_test
  gtest gtest_main gmock
)

add_executable(
  rdb_test
  rdb_test.cpp
)
target_link_libraries(
  rdb_test
  gtest gtest_main gmock
  redis_server
)
target_compile_definitions(rdb_test PUBLIC RDB_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/dump.rdb")


include(GoogleTest)
gtest_discover_tests(parsing_test)
gtest_discover_tests(rdb_test)