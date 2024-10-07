#ifndef REDIS_SERVER_HPP
#define REDIS_SERVER_HPP
#include "Config.hpp"
#include "Types.hpp"
#include <TCPClient.hpp>
#include <asio.hpp>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
class TCPConnection;

namespace Redis {

class Server : public std::enable_shared_from_this<Server> {
public:
  using SharedPtr = std::shared_ptr<Redis::Server>;
  using Reply = std::vector<std::string>;

  /**
   * @brief Construct a new Redis Server object.
   *
   * This constructor initializes a Redis server instance with default
   * configuration. It sets up the server to listen on the default Redis port
   * (6379) and initializes the server's internal state, including the command
   * lookup table and database.
   *
   * @param port The port number on which the server will listen. Defaults to
   * 6379.
   */
  Server(int port = 6379);

  /**
   * @brief Construct a new Redis Server object as a replica.
   *
   * This constructor initializes a Redis server instance that acts as a replica
   * of a master server. It sets up the server to listen on the specified port
   * and attempts to establish a connection with the master server.
   *
   * @param port The port number on which this replica server will listen.
   * @param masterIp The IP address of the master Redis server.
   * @param masterPort The port number of the master Redis server.
   * @param ioContext The asio::io_context object to be used for asynchronous
   * operations.
   *
   * @throws std::runtime_error If the connection to the master server cannot be
   * established.
   */
  Server(int port, std::string masterIp, int masterPort,
         asio::io_context &ioContext);

  virtual ~Server() = default;

  /**
   * @brief Giving a message from redis client, parse it and return the expected
   * response.
   *
   * @param message Received message from the redis client through the open
   * socket.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return std::optional<std::string> The response to the given message,
   * std::nullopt if the message parsing failed.
   */
  std::optional<Reply> handleRequest(const std::string &message,
                                     std::size_t clientId = -1);

  /**
   * @brief Is this server a replica of another master redis server.
   *
   * @return Bool True if it's a replica.
   */
  bool isReplica() const {
    return masterIp.has_value() && masterPort.has_value();
  }

  /**
   * @brief Register a new client connection with the server.
   *
   * This function adds a new client connection to the server's list of clients.
   * It uses a weak pointer to avoid circular references and allow for automatic
   * cleanup when the connection is closed.
   *
   * @param clientPtr A weak pointer to the TCPConnection object representing
   * the client.
   * @return std::size_t The index of the newly registered client in the
   * server's client list.
   */
  std::size_t registerClient(std::weak_ptr<TCPConnection> clientPtr);

private:
  /**
   * @brief Initialize the server.
   *
   * This function performs the following tasks:
   * 1. Initializes the command lookup table (cmdsLUT).
   * 2. Loads the RDB file from the configured path.
   * 3. If an RDB file is found and successfully parsed, it populates the
   * server's database with the loaded data.
   *
   * The function is called in the constructor to set up the server's initial
   * state.
   */

  void init();

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
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return std::optional<Reply> The response to this command.
   */
  std::optional<Reply> handleCommands(const std::vector<std::string> &commands,
                                      std::size_t clientId);

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
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return std::string Server response to the command.
   */
  Reply pingCommand(const std::vector<std::string> &commands,
                    std::size_t clientId);

  /**
   * @brief Parse a `ECHO` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply echoCommand(const std::vector<std::string> &commands,
                    std::size_t clientId);

  /**
   * @brief Parse a `GET` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply getCommand(const std::vector<std::string> &commands,
                   std::size_t clientId);

  /**
   * @brief Parse a `SET` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply setCommand(const std::vector<std::string> &commands,
                   std::size_t clientId);

  /**
   * @brief Parse a `CONFIG` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply configCommand(const std::vector<std::string> &commands,
                      std::size_t clientId);

  /**
   * @brief Parse a `KEYS` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply keysCommand(const std::vector<std::string> &commands,
                    std::size_t clientId);

  /**
   * @brief Parse a `INFO` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply infoCommand(const std::vector<std::string> &commands,
                    std::size_t clientId);

  /**
   * @brief Parse a `REPLCONF` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply replconfCommand(const std::vector<std::string> &commands,
                        std::size_t clientId);

  /**
   * @brief Parse a `PSYNC` command from redis client.
   *
   * @param commands The redis command and it's argument.
   * @param clientId The unique identifier of the client sending the command.
   *                 This is used to track client-specific state and for
   *                 operations that may differ based on the client's context.
   * @return Reply Server response to the command.
   */
  Reply psyncCommand(const std::vector<std::string> &commands,
                     std::size_t clientId);

  /**
   * @brief Handshake with the master server.
   *
   * @return True if the handshake is successful, false otherwise.
   */
  bool handShakeMaster(asio::io_context &);

  /**
   * @brief Propagate commands to all connected replicas.
   *
   * This function sends the given commands to all replica servers that are
   * currently connected to this Redis server. It's typically used after
   * executing a write operation to ensure that all replicas stay in sync with
   * the master.
   *
   * @param commands A vector of strings representing the Redis command and its
   * arguments that should be propagated to the replicas.
   */
  void propagateToReplicas(const std::vector<std::string> &commands);

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
      std::string,
      std::function<Reply(const std::vector<std::string> &, std::size_t)>>
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
   * is a replica.
   */
  std::shared_ptr<TCPClient> replicaClient;

  /**
   * @brief A vector of weak pointers to TCPConnection objects representing
   * client connections to this server.
   *
   * This container stores weak pointers to avoid circular references. When a
   * client disconnects, its corresponding weak pointer will automatically
   * expire, allowing for proper cleanup.
   */
  std::vector<std::weak_ptr<TCPConnection>> clients;

  /**
   * @brief A vector of weak pointers to TCPConnection objects representing
   * replica servers connected to this server.
   *
   * This container is used when this server acts as a master, storing
   * connections to its replicas. Weak pointers are used to prevent circular
   * references and allow for automatic cleanup when a replica disconnects.
   */
  std::vector<std::weak_ptr<TCPConnection>> replicas;
};
} // namespace Redis

#endif
