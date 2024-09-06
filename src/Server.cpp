#include "TCPServer.hpp"
#include "quill_wrapper/overwrite_macros.h"
#include "quill_wrapper/quill_wrapper.h"
#include <arpa/inet.h>
#include <asio.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
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
  setup_quill("redis_server.log");
  // Change the LogLevel to print everything
  global_logger_a->set_log_level(quill::LogLevel::TraceL3);
  LOG_INFO("Starting the server ..");
  try {
    asio::io_context io_context;
    TCPServer server(io_context);
    server.start();
    io_context.run();
  } catch (std::exception &e) {
    LOG_ERROR("Error {}", e.what());
  }
  return 0;
}
