#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP
#include "Config.hpp"
#include "Types.hpp"
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
namespace Redis {

class Server {
public:
  using SharedPtr = std::shared_ptr<Redis::Server>;
  Server();
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
  Database data_;
  std::mutex dataMutex_;
  Config config_;
};
} // namespace Redis

#endif
