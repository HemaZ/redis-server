#include "RESP/Constants.hpp"
#include "RedisServer.hpp"
#include <TCPClient.hpp>
#include <TCPServer.hpp>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>
using namespace std::chrono_literals;
TEST(TCP_CLIENT, PING) {
  Redis::Server::SharedPtr redisServer = std::make_shared<Redis::Server>();
  asio::io_context io_context;
  TCPServer server(io_context, 12345, redisServer);
  server.start();
  std::thread t([&] { io_context.run(); });

  auto client = TCPClient::create(io_context, "localhost", 12345);
  auto error = client->send("*1\r\n$4\r\nPING\r\n");
  client->setCallback(
      [](const std::string &msg) { ASSERT_EQ(msg, "+PONG\r\n"); });
  client->listen();
  ASSERT_FALSE(error);
  std::string message;
  error = client->read(message);
  ASSERT_FALSE(error);
  ASSERT_EQ(message, "+PONG\r\n");
  usleep(1000000);
  io_context.stop();
  t.join();
}
