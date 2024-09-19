#include "RESP/Constants.hpp"
#include "RedisServer.hpp"
#include <gtest/gtest.h>

TEST(REDIS_SERVER, PING) {
  Redis::Server server;
  auto res = server.handleRequest("*1\r\n$4\r\nping\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, std::string("+PONG\r\n"));

  res = server.handleRequest("*1\r\n$4\r\nPING\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, std::string("+PONG\r\n"));
}

TEST(REDIS_SERVER, ECHO) {
  Redis::Server server;
  auto res = server.handleRequest("*2\r\n$4\r\necho\r\n$3\r\nhey\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, std::string("+hey\r\n"));
}

TEST(REDIS_SERVER, SETGET) {
  Redis::Server server;

  // Create a key foo and set value to bar
  auto res =
      server.handleRequest("*3\r\n$4\r\nset\r\n$3\r\nfoo\r\n$3\r\nbar\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, std::string("+OK\r\n"));

  // get the value of the key foo
  res = server.handleRequest("*2\r\n$4\r\nget\r\n$3\r\nfoo\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, std::string("$3\r\nbar\r\n"));

  // Get a non existing key
  res = server.handleRequest("*2\r\n$4\r\nget\r\n$3\r\nbaz\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, RESP::NullBString);
}