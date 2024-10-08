#include "RESP/Constants.hpp"
#include "RedisServer.hpp"
#include <gtest/gtest.h>
using Reply = Redis::Server::Reply;

TEST(REDIS_SERVER, PING) {
  Redis::Server server;
  auto res = server.handleRequest("*1\r\n$4\r\nping\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, Reply({"+PONG\r\n"}));

  res = server.handleRequest("*1\r\n$4\r\nPING\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, Reply({"+PONG\r\n"}));
}

TEST(REDIS_SERVER, ECHO) {
  Redis::Server server;
  auto res = server.handleRequest("*2\r\n$4\r\necho\r\n$3\r\nhey\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, Reply({"+hey\r\n"}));
}

TEST(REDIS_SERVER, SETGET) {
  Redis::Server server;

  // Create a key foo and set value to bar
  auto res =
      server.handleRequest("*3\r\n$4\r\nset\r\n$3\r\nfoo\r\n$3\r\nbar\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, Reply({"+OK\r\n"}));

  // get the value of the key foo
  res = server.handleRequest("*2\r\n$4\r\nget\r\n$3\r\nfoo\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, Reply({"$3\r\nbar\r\n"}));

  // Get a non existing key
  res = server.handleRequest("*2\r\n$4\r\nget\r\n$3\r\nbaz\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, Reply{RESP::NullBString});
}

TEST(REDIS_SERVER, INFO) {
  Redis::Server server;
  auto res = server.handleRequest("*2\r\n$4\r\nINFO\r\n$11\r\replication\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_NE(res->at(0).find("$11\r\nrole:master\r\n"), std::string::npos);

  // This should return only replication info for now
  // In the future we should check on the other info
  res = server.handleRequest("*1\r\n$4\r\nINFO\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_NE(res->at(0).find("$11\r\nrole:master\r\n"), std::string::npos);

  // Create a replica server
  // Redis::Server replica(6379, "localhost", 6379);
  // ASSERT_TRUE(replica.isReplica());
  // res =
  // replica.handleRequest("*2\r\n$4\r\nINFO\r\n$11\r\replication\r\n");
  // ASSERT_TRUE(res.has_value());
  // EXPECT_NE(res->find("$10\r\nrole:slave\r\n"), std::string::npos);
}

TEST(REDIS_SERVER, PSYNC) {
  Redis::Server server;
  auto res =
      server.handleRequest("*3\r\n$4\r\npsync\r\n$1\r\n1\r\n$1\r\n2\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_NE(res->at(0).find("+FULLRESYNC"), std::string::npos);
}

TEST(REDIS_SERVER, REPLCONF) {
  Redis::Server server;
  auto res =
      server.handleRequest("*3\r\n$4\r\nreplconf\r\n$1\r\n1\r\n$1\r\n2\r\n");
  ASSERT_TRUE(res.has_value());
  EXPECT_EQ(*res, Reply({"+OK\r\n"}));
}
