#include "RedisServer.hpp"
#include "Helper.hpp"
#include "Logging.hpp"
#include "RESP/Parsing.hpp"
namespace Redis {

std::optional<std::string>
Server::handleSingleCommand(const std::string &message) {
  LOG_DEBUG("Received Command {} ", message);
  if (message == "ping") {
    return "+PONG\r\n";
  }
  if (message == "COMMAND") {
    return "+PONG\r\n";
  }
  return std::nullopt;
}

std::optional<std::string>
Server::handleMultipleCommands(const std::vector<std::string> &commands) {
  if (commands[0] == "echo" || commands[0] == "ECHO") {
    return "+" + commands[1] + "\r\n";
  }
  return std::nullopt;
}

std::optional<std::string>
Server::handleCommands(const std::vector<std::string> &commands) {
  LOG_DEBUG("Commands Received {}", commands);
  if (commands.size() == 1) {
    return handleSingleCommand(commands[0]);
  }
  return handleMultipleCommands(commands);
}

std::optional<std::string> Server::handleRequest(const std::string &message) {
  if (message.empty()) {
    LOG_DEBUG("Recived an empty message");
    return "\r\n";
  }
  if (message[0] != RESP::DataType::ARRAY) {
    LOG_DEBUG("Recived a non array command {}", message);
    return "\r\n";
  }
  std::optional<std::vector<std::string>> commands = RESP::parseArray(message);
  if (!commands) {
    LOG_DEBUG("Recived a non array command {}", message);
    return std::nullopt;
  }
  return handleCommands(*commands);
}
} // namespace Redis