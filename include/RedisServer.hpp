#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP
#include <optional>
#include <string>
#include <vector>
namespace Redis {
class Server {
public:
  Server() {}
  std::string handleSingleCommand(const std::string &message);
  std::optional<std::string> handleRequest(const std::string &message);

private:
  std::string handleCommands(const std::vector<std::string> &commands);
};
} // namespace Redis

#endif
