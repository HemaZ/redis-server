#include "Logging.hpp"
#include <gtest/gtest.h>
int main(int argc, char **argv) {
  // Logging setup
  setup_quill("tests.log");
  global_logger_a->set_log_level(quill::LogLevel::TraceL3);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}