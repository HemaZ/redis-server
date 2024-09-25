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

TEST(REDIS_SERVER, INFO) {
  Redis::Server server;
  auto res = server.handleRequest("*2\r\n$4\r\nINFO\r\n$11\r\replication\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_NE(res->find("$11\r\nrole:master\r\n"), std::string::npos);

  // This should return only replication info for now
  // In the future we should check on the other info
  res = server.handleRequest("*1\r\n$4\r\nINFO\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_NE(res->find("$11\r\nrole:master\r\n"), std::string::npos);

  // Create a replica server
  Redis::Server replica(6379, "192.168.1.1", 7534);
  ASSERT_TRUE(replica.isReplica());
  res = replica.handleRequest("*2\r\n$4\r\nINFO\r\n$11\r\replication\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_NE(res->find("$10\r\nrole:slave\r\n"), std::string::npos);
}
