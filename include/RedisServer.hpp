#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP
#include "Config.hpp"
#include "TCPClient.hpp"
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
  using Reply = std::vector<std::string>;

  /**
   * @brief Construct a new Redis Server.
   * @param port The server port.
   * @param masterIp Optional master server ip to create a replica.
   * @param masterPort Optional master server port to create a replica.
   */
  Server(int port = 6379, std::optional<std::string> masterIp = std::nullopt,
         std::optional<int> masterPort = std::nullopt);

  virtual ~Server();
  /**
   * @brief Giving a message from redis client, parse it and return the expected
   * response.
   *
   * @param message Received message from the redis client through the open
   * socket.
   * @return std::optional<std::string> The response to the given message,
   * std::nullopt if the message parsing failed.
   */
  std::optional<Reply> handleRequest(const std::string &message);

  /**
   * @brief Is this server a replica of another master redis server.
   *
   * @return Bool True if it's a replica.
   */
  bool isReplica() const {
    return masterIp.has_value() && masterPort.has_value();
  }

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
   * @return std::optional<Reply> The response to this command.
   */
  std::optional<Reply> handleCommands(const std::vector<std::string> &commands);

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
  Reply pingCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `ECHO` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply echoCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `GET` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply getCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `SET` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply setCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `CONFIG` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply configCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `KEYS` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply keysCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `INFO` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply infoCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `REPLCONF` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply replconfCommand(const std::vector<std::string> &commands);

  /**
   * @brief Parse a `PSYNC` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @return Reply Server response to the command.
   */
  Reply psyncCommand(const std::vector<std::string> &commands);

  /**
   * @brief Handshake with the master server.
   *
   * @return True if the handshake is successful, false otherwise.
   */
  bool handShakeMaster();

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
  std::unordered_map<std::string,
                     std::function<Reply(const std::vector<std::string> &)>>
      cmdsLUT;

  /**
   * @brief The port number on which this Redis server is listening.
   */
  int port;

  /**
   * @brief The IP address of the master Redis server, if this server is a
   * replica. This is std::nullopt if this server is not a replica.
   */
  std::optional<std::string> masterIp;

  /**
   * @brief The port number of the master Redis server, if this server is a
   * replica. This is std::nullopt if this server is not a replica.
   */
  std::optional<int> masterPort;

  /**
   * @brief The replication ID of the master server.
   * This is used to identify the replication stream.
   */
  std::string masterReplId;

  /**
   * @brief The current replication offset of this server.
   * This represents how much of the master's replication stream has been
   * processed.
   */
  int masterReplOffset = 0;

  /**
   * @brief A TCP client used to connect to the master server when this server
   * is a replica. This is std::nullopt if this server is not a replica.
   */
  std::optional<TCPClient> replicaClient = std::nullopt;

  /**
   * @brief The I/O context for asynchronous operations.
   * This is used by Asio for managing asynchronous operations.
   */
  asio::io_context ioContext;

  /**
   * @brief A thread dedicated to running the I/O context.
   * This allows asynchronous operations to be processed independently of the
   * main thread.
   */
  std::thread ioThread;
};
} // namespace Redis

#endif
