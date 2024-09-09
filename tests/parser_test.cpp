#include <RESP/Parsing.hpp>
#include <gtest/gtest.h>
#include <optional>
// Demonstrate some basic assertions.
TEST(RESP_PARSING, BasicParsing) {
  auto val = RESP::parseArray("");
  EXPECT_EQ(val, std::nullopt);

  auto val2 = RESP::parseArray("+");
  EXPECT_EQ(val2, std::nullopt);

  auto val3 = RESP::parseArray("*");
  EXPECT_NE(val3, std::nullopt);
}

TEST(RESP_PARSING, BulkString) {
  auto val = RESP::parseBString("");
  EXPECT_EQ(val, std::nullopt);

  auto val2 = RESP::parseBString("+");
  EXPECT_EQ(val2, std::nullopt);

  auto val3 = RESP::parseBString("$4\r\necho\r\n");
  EXPECT_EQ(val3, "echo");

  auto val4 = RESP::parseBString("4\r\necho\r\n");
  EXPECT_EQ(val4, std::nullopt);

  auto val5 = RESP::parseBString("$\r\necho\r\n");
  EXPECT_EQ(val5, std::nullopt);
}

TEST(RESP_PARSING, Array) {
  auto val = RESP::parseArray("*2\r\n$4\r\nECHO\r\n$3\r\nhey\r\n");
  EXPECT_TRUE(val.has_value());
  EXPECT_EQ(val->size(), 2);
  EXPECT_EQ(val->at(0), "ECHO");
  EXPECT_EQ(val->at(1), "hey");
}