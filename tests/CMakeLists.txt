enable_testing()

add_executable(
  parsing_test
  parser_test.cpp
  test_main.cpp
)
target_link_libraries(
  parsing_test
  gtest gmock quill_wrapper_recommended
)

add_executable(
  rdb_test
  rdb_test.cpp
  test_main.cpp
)
target_link_libraries(
  rdb_test
  gtest gmock
  redis_server quill_wrapper_recommended
)
target_compile_definitions(rdb_test PUBLIC RDB_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/dump.rdb")
target_compile_definitions(rdb_test PUBLIC RDB_EXP_FILE_PATH="${CMAKE_CURRENT_SOURCE_DIR}/data/dump-exp.rdb")

add_executable(
  server_test
  redis_server_tests.cpp test_main.cpp
)
target_link_libraries(
  server_test
  gtest gmock
  redis_server quill_wrapper_recommended
)


include(GoogleTest)
gtest_discover_tests(parsing_test)
gtest_discover_tests(rdb_test)
gtest_discover_tests(server_test)