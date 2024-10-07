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
  std::optional<std::string> masterIp;
  std::optional<int> masterPort;
  if (result.count("replicaof")) {
    std::string replica = result["replicaof"].as<std::string>();
    std::size_t index = replica.find(" ");
    if (index == std::string::npos) {
      LOG_ERROR("Replica should have the format ServerIP ServerPort");
      exit(EXIT_FAILURE);
    }
    masterIp = replica.substr(0, index);
    masterPort = std::stoi(replica.substr(index, replica.size() - index));
    LOG_INFO("Starting a replica server on {}:{}", *masterIp, *masterPort);
  }

  // Change the LogLevel to print everything
  if (debug)
    global_logger_a->set_log_level(quill::LogLevel::TraceL3);

  // Creating the server
  asio::io_context io_context;
  try {
    Redis::Server::SharedPtr redisServer;
    if (masterIp.has_value()) {
      redisServer = std::make_shared<Redis::Server>(port, *masterIp,
                                                    *masterPort, io_context);
    } else {
      redisServer = std::make_shared<Redis::Server>(port);
    }

    LOG_INFO("Starting the server on port {}", port);
    TCPServer server(io_context, port, redisServer);
    server.start();
    io_context.run();
  } catch (std::exception &e) {
    LOG_ERROR("Error {}", e.what());
  }
  return 0;
}
