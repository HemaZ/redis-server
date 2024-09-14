#include "RedisServer.hpp"
#include "Helper.hpp"
#include "Logging.hpp"
#include "RESP/Parsing.hpp"
namespace Redis {

std::optional<std::string> Server::getValue(const std::string &key) {
  if (data_.contains(key)) {
    const Record &record = data_.at(key);
    if (!data_.at(key).expired()) {
      return data_.at(key).data;
    } else {
      data_.erase(key);
    }
  }
  return std::nullopt;
}

void Server::setValue(const std::string &key, const std::string &value,
                      std::optional<int> expiry) {
  Record newRecord;
  newRecord.data = value;
  if (expiry) {
    newRecord.setExpiry(*expiry);
  }
  std::lock_guard<std::mutex> lock(dataMutex_);
  data_[key] = newRecord;
}

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
  if (commands[0] == "get" || commands[0] == "GET") {
    LOG_DEBUG("Getting the value {}", commands[1]);
    auto val = getValue(commands[1]);
    if (val) {
      return RESP::toBString(*val);
    } else {
      return "$-1\r\n";
    }
    return "+" + commands[1] + "\r\n";
  }
  if (commands[0] == "set" || commands[0] == "SET") {
    return setCommand(commands);
  }
  return std::nullopt;
}
std::string Server::setCommand(const std::vector<std::string> &commands) {
  if (commands.size() != 3 && commands.size() != 5) {
    return "$-1\r\n";
  }
  std::optional<int> expiry;
  if (commands.size() == 5) {
    try {
      expiry = std::stoi(commands[4]);
      LOG_INFO("Expiry time is {}ms", *expiry);
    } catch (std::invalid_argument const &ex) {
      LOG_ERROR("Expiry time is invalid {}", ex.what());
    }
  }
  LOG_DEBUG("Setting the key {} to {}", commands[1], commands[2]);
  setValue(commands[1], commands[2], expiry);
  return "+OK\r\n";
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