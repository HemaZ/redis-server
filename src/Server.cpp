#include "Logging.hpp"
#include "TCPServer.hpp"
#include <arpa/inet.h>
#include <asio.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cxxopts.hpp>
#include <iostream>
#include <netdb.h>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

using asio::ip::tcp;

int main(int argc, char **argv) {
  // Logging setup
  setup_quill("redis_server.log");

  // arguments parsing
  cxxopts::Options options("Redis-Server",
                           "A c++ implementation for Redis server");
  // clang-format off
  options.add_options()("d,debug", "Enable debugging")
  ("p,port", "Port number", cxxopts::value<int>()->default_value("6379"))
  ("r,replicaof", "Replica of the master server", cxxopts::value<std::string>())
  ("h,help", "Print usage");
  // clang-format on

  auto result = options.parse(argc, argv);
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }
  bool debug = result["debug"].as<bool>();
  int port = result["port"].as<int>();
  std::optional<std::string> master;
  if (result.count("replicaof")) {
    master = result["replicaof"].as<std::string>();
  }

  // Change the LogLevel to print everything
  if (debug)
    global_logger_a->set_log_level(quill::LogLevel::TraceL3);

  // Creating the server
  try {
    Redis::Server::SharedPtr redisServer = std::make_shared<Redis::Server>();
    LOG_INFO("Starting the server on port {}", port);
    asio::io_context io_context;
    TCPServer server(io_context, port, redisServer);
    server.start();
    io_context.run();
  } catch (std::exception &e) {
    LOG_ERROR("Error {}", e.what());
  }
  return 0;
}
