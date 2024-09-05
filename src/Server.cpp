#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/Logger.h"
#include "quill/sinks/ConsoleSink.h"
#include <arpa/inet.h>
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

static quill::Logger *get_logger() {
  // Backend
  quill::BackendOptions backend_options;
  quill::Backend::start(backend_options);

  // Frontend
  auto console_sink =
      quill::Frontend::create_or_get_sink<quill::ConsoleSink>("sink_id_1");
  quill::Logger *logger =
      quill::Frontend::create_or_get_logger("root", std::move(console_sink));
  // Change the LogLevel to print everything
  logger->set_log_level(quill::LogLevel::TraceL3);
  return logger;
}

void handle_client(int client_fd) {
  while (true) {
    char buffer[256];
    size_t bytesReceived = recv(client_fd, buffer, 256, 0);
    if (bytesReceived < 0) {
      break;
    }
    LOG_DEBUG(get_logger(), "Recived message {}", buffer);
    send(client_fd, "+PONG\r\n", 7, 0);
  }
}
int main(int argc, char **argv) {

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 6379\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  std::vector<std::thread> runningThreads;
  LOG_INFO(get_logger(), "Waiting for clients to connect");

  while (true) {
    int client = accept(server_fd, (struct sockaddr *)&client_addr,
                        (socklen_t *)&client_addr_len);
    LOG_INFO(get_logger(), "New client connected with id {}", client);
    runningThreads.emplace_back(handle_client, client);
  }

  close(server_fd);

  return 0;
}
