#include "RESP/Constants.hpp"
#include "RedisServer.hpp"
#include <TCPClient.hpp>
#include <TCPServer.hpp>
#include <gtest/gtest.h>
#include <thread>

TEST(TCP_CLIENT, PING) {
  Redis::Server::SharedPtr redisServer = std::make_shared<Redis::Server>();
  asio::io_context io_context;
  TCPServer server(io_context, 12345, redisServer);
  server.start();
  std::thread t([&] { io_context.run(); });

  TCPClient client(io_context, "localhost", 12345);
  auto error = client.send("*1\r\n$4\r\nPING\r\n");
  ASSERT_FALSE(error);
  std::string message;
  error = client.read(message);
  ASSERT_FALSE(error);
  ASSERT_EQ(message, "+PONG\r\n");
  io_context.stop();
  t.join();
}
