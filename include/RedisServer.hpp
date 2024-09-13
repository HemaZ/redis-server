#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP
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
  Server() {}
  std::optional<std::string> handleSingleCommand(const std::string &message);
  std::optional<std::string> handleRequest(const std::string &message);
  std::optional<std::string>
  handleMultipleCommands(const std::vector<std::string> &commands);

private:
  std::optional<std::string> getValue(const std::string &key) const;
  void setValue(const std::string &key, const std::string &value);
  std::optional<std::string>
  handleCommands(const std::vector<std::string> &commands);
  std::unordered_map<std::string, std::string> data_;
  std::mutex dataMutex_;
};
} // namespace Redis

#endif
