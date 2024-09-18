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
  // arguments parsing
  cxxopts::Options options("Redis-Server",
                           "A c++ implememntation for Redis server");
  options.add_options()("d,debug", "Enable debugging")(
      "p,port", "Port number",
      cxxopts::value<int>()->default_value("6379"))("h,help", "Print usage");
  auto result = options.parse(argc, argv);
  if (result.count("help")) {
    std::cout << options.help() << std::endl;
    exit(0);
  }
  bool debug = result["debug"].as<bool>();
  int port = result["port"].as<int>();

  // Logging setup
  setup_quill("redis_server.log");
  // Change the LogLevel to print everything
  if (debug)
    global_logger_a->set_log_level(quill::LogLevel::TraceL3);

  // Creating the server
  try {
    LOG_INFO("Starting the server on port {}", port);
    asio::io_context io_context;
    TCPServer server(io_context, port);
    server.start();
    io_context.run();
  } catch (std::exception &e) {
    LOG_ERROR("Error {}", e.what());
  }
  return 0;
}
