#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP
#include "Config.hpp"
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace Redis {

struct Record {
  std::string data;
  std::optional<std::chrono::time_point<std::chrono::system_clock>> expiry;
  void setExpiry(int milliseconds) {
    expiry = std::chrono::system_clock::now() +
             std::chrono::milliseconds(milliseconds);
  }
  bool expired() const {
    if (expiry == std::nullopt) {
      return false;
    }
    return *expiry <= std::chrono::system_clock::now();
  }
};

class Server {
public:
  using SharedPtr = std::shared_ptr<Redis::Server>;
  Server() {}
  std::optional<std::string> handleSingleCommand(const std::string &message);
  std::optional<std::string> handleRequest(const std::string &message);
  std::optional<std::string>
  handleMultipleCommands(const std::vector<std::string> &commands);

private:
  std::optional<std::string> getValue(const std::string &key);
  void setValue(const std::string &key, const std::string &value,
                std::optional<int> expiry = std::nullopt);
  std::optional<std::string>
  handleCommands(const std::vector<std::string> &commands);
  std::string setCommand(const std::vector<std::string> &commands);
  std::string configCommand(const std::vector<std::string> &commands);
  std::unordered_map<std::string, Record> data_;
  std::mutex dataMutex_;
  Config config_;
};
} // namespace Redis

#endif
