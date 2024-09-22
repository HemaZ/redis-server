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
  /**
   * @brief Construct a new Redis Server.
   *
   */
  Server();

  /**
   * @brief Giving a message from redis client, parse it and return the expected
   * response.
   *
   * @param message Received message from the redis client through the open
   * socket.
   * @return std::optional<std::string> The response to the given message,
   * std::nullopt if the message parsing failed.
   */
  std::optional<std::string> handleRequest(const std::string &message);

private:
  /**
   * @brief Initialize the cmdsLUT which holds redis command as a key
   * and the corresponding parsing function as value.
   */
  void initCmdsLUT();

  /**
   * @brief Given list of command and arguments. parse it and return the
   * response.
   *
   * @param commands List of redis command with it's arguments.
   * @return std::optional<std::string> The response to this command.
   */
  std::optional<std::string>
  handleCommands(const std::vector<std::string> &commands);

  /**
   * @brief Get a stored value giving the key.
   *
   * @param key Key as string.
   * @return std::optional<std::string> std::nullopt if the key doesn't exist or
   * the value is expired.
   */
  std::optional<std::string> getValue(const std::string &key);

  /**
   * @brief Create a new record in the database giving the key and value and an
   * optional expiry time.
   *
   * @param key The new record key.
   * @param value The new record value.
   * @param expiry Expiry time in miliseconds.
   */
  void setValue(const std::string &key, const std::string &value,
                std::optional<int> expiry = std::nullopt);

  /**
   * @brief Parse a `PING` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return std::string Server response to the command.
   */
  std::string pingCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `ECHO` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return std::string Server response to the command.
   */
  std::string echoCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `GET` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return std::string Server response to the command.
   */
  std::string getCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `SET` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return std::string Server response to the command.
   */
  std::string setCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `CONFIG` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return std::string Server response to the command.
   */
  std::string configCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `KEYS` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return std::string Server response to the command.
   */
  std::string keysCommand(const std::vector<std::string> &commands);

  /**
   * @brief The main server database. Reading from the database should be thread
   * safe. Inserting in the database should only be done through the function
   * @sa setValue.
   */
  Database data_;

  /**
   * @brief Mutex to protect writing to the database.
   */
  std::mutex dataMutex_;

  /**
   * @brief The server config.
   *
   */
  Config config_;

  /**
   * @brief Lookup table for redis command and the corresponding function to
   * handle this command.
   */
  std::unordered_map<
      std::string, std::function<std::string(const std::vector<std::string> &)>>
      cmdsLUT;
};
} // namespace Redis

#endif
