#include "RedisServer.hpp"
#include "Helper.hpp"
#include "Logging.hpp"
#include "RDBFile.hpp"
#include "RESP/RESP.hpp"
#include <TCPClient.hpp>
#include <TCPConnection.hpp>
#include <asio.hpp>
#include <filesystem>
#include <regex>
#include <thread>
namespace fs = std::filesystem;

namespace Redis {

Server::Server(int port) : port(port) { init(); }

Server::Server(int port, std::string masterIp, int masterPort,
               asio::io_context &ioContext)
    : port(port), masterIp(masterIp), masterPort(masterPort),
      masterReplId(randomString(40)) {
  init();
  if (isReplica() && !handShakeMaster(ioContext)) {
    throw std::runtime_error("Couldn't connect to the master server");
  }
}

std::size_t Server::registerClient(std::weak_ptr<TCPConnection> clientPtr) {
  clients.push_back(clientPtr);
  return clients.size() - 1;
}

void Server::init() {
  initCmdsLUT();
  fs::path rdbFilePath = fs::path(config_.dir) / fs::path(config_.dbfilename);
  auto rdbDatabase = parseRDBFile(rdbFilePath);
  if (rdbDatabase) {
    LOG_INFO("Loaded RDB file from the path {} with {} records",
             config_.dbfilename, rdbDatabase->size());
    data_.insert(rdbDatabase->begin(), rdbDatabase->end());
  }
}

void Server::propagateToReplicas(const std::vector<std::string> &commands) {
  std::string command = RESP::toStringArray(commands);
  for (const auto &replica : replicas) {
    if (auto ptr = replica.lock(); ptr != nullptr) {
      LOG_INFO("Sending a message to the replica ");
      ptr->send_message(command);
    }
  }
}

bool Server::handShakeMaster(asio::io_context &ioContext) {
  replicaClient = TCPClient::create(ioContext, *masterIp, *masterPort);
  // ioThread = std::thread([&] { ioContext.run(); });
  LOG_INFO("Pinging the master server on {} {}", *masterIp, *masterPort);
  auto readWrite = [&](const std::string_view &cmd,
                       const std::string_view &expectedReply) -> bool {
    auto error = replicaClient->send(std::string(cmd));
    if (error) {
      LOG_ERROR("Error Sending ping to the master {}", error.message());
      return false;
    }
    std::string reply;
    error = replicaClient->read(reply);
    if (error) {
      LOG_ERROR("Error getting reply from the master {}", error.message());
      return false;
    }
    LOG_INFO("Received reply from master {}", reply);
    return expectedReply == reply;
  };

  if (!readWrite("*1\r\n$4\r\nPING\r\n", "+PONG\r\n")) {
    return false;
  }
  if (!readWrite(RESP::toStringArray(
                     {"REPLCONF", "listening-port", std::to_string(port)}),
                 "+OK\r\n")) {
    return false;
  }
  if (!readWrite(RESP::toStringArray({"REPLCONF", "capa", "psync2"}),
                 "+OK\r\n")) {
    return false;
  }

  readWrite(RESP::toStringArray({"PSYNC", "?", "-1"}), "");

  std::string rdbFile;
  auto error = replicaClient->read(rdbFile);
  if (error) {
    LOG_ERROR("Error getting rdb file from the master {}", error.message());
    return false;
  }

  LOG_INFO("RDB file received from master, {}", rdbFile);
  fs::path rdbFilePath = fs::temp_directory_path() / fs::path("replica.rdb");
  // Open a new binary file to write the RDB content
  std::ofstream outFile(rdbFilePath, std::ios::binary);
  if (!outFile) {
    LOG_ERROR("Failed to create {} file", rdbFilePath.string());
    return false;
  }
  std::string rdbFileContent = rdbFile.substr(rdbFile.find("\r\n") + 2);
  // Write the content of rdbFile to the new file
  outFile.write(rdbFileContent.data(), rdbFileContent.size());
  if (!outFile) {
    LOG_ERROR("Failed to write RDB data to replica.rdb");
    return false;
  }
  outFile.close();
  LOG_INFO("RDB file successfully written to replica.rdb");

  auto rdbDatabase = parseRDBFile(rdbFilePath);
  if (rdbDatabase) {
    LOG_INFO("Loaded Replica RDB file from the path {} with {} records",
             config_.dbfilename, rdbDatabase->size());
    data_.insert(rdbDatabase->begin(), rdbDatabase->end());
  }
  replicaClient->setCallback(
      [this](const std::string &msg) { handleRequest(msg, 0); });
  replicaClient->listen();
  // asio::post(ioContext, [this]() { replicaClient->listen(); });
  // replicaClient->listen();

  // TODO check why the client is not getting the master messages
  // replicaThread = std::thread([&] {
  //   while (true) {
  //     std::string msg;
  //     replicaClient->read(msg);
  //     LOG_INFO("Received message from master {}", msg);
  //     handleRequest(msg, 0);
  //     // asio::post()
  //     // TODO optimize this part, use the iocontext to post a lambda
  //     // which calls async_read_until in the client
  //   }
  // });

  return true;
}

void Server::initCmdsLUT() {
  cmdsLUT["ping"] = std::bind(&Server::pingCommand, this, std::placeholders::_1,
                              std::placeholders::_2);
  cmdsLUT["command"] = std::bind(&Server::pingCommand, this,
                                 std::placeholders::_1, std::placeholders::_2);
  cmdsLUT["echo"] = std::bind(&Server::echoCommand, this, std::placeholders::_1,
                              std::placeholders::_2);
  cmdsLUT["get"] = std::bind(&Server::getCommand, this, std::placeholders::_1,
                             std::placeholders::_2);
  cmdsLUT["set"] = std::bind(&Server::setCommand, this, std::placeholders::_1,
                             std::placeholders::_2);
  cmdsLUT["config"] = std::bind(&Server::configCommand, this,
                                std::placeholders::_1, std::placeholders::_2);
  cmdsLUT["keys"] = std::bind(&Server::keysCommand, this, std::placeholders::_1,
                              std::placeholders::_2);
  cmdsLUT["info"] = std::bind(&Server::infoCommand, this, std::placeholders::_1,
                              std::placeholders::_2);
  cmdsLUT["replconf"] = std::bind(&Server::replconfCommand, this,
                                  std::placeholders::_1, std::placeholders::_2);
  cmdsLUT["psync"] = std::bind(&Server::psyncCommand, this,
                               std::placeholders::_1, std::placeholders::_2);
  LOG_DEBUG("Init CMDS LUT with {} commands", cmdsLUT.size());
}

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

std::optional<Server::Reply>
Server::handleCommands(const std::vector<std::string> &commands,
                       std::size_t clientId) {
  std::string command = strTolower(commands[0]);
  try {
    return cmdsLUT.at(command)(commands, clientId);
  } catch (const std::out_of_range &e) {
    LOG_ERROR("Unrecognised command {}", command);
    return std::nullopt;
  }
}

Server::Reply Server::pingCommand(const std::vector<std::string> &commands,
                                  std::size_t clientId) {
  return Server::Reply{"+PONG\r\n"};
}

Server::Reply Server::echoCommand(const std::vector<std::string> &commands,
                                  std::size_t clientId) {
  return Server::Reply{"+" + commands[1] + "\r\n"};
}

Server::Reply Server::getCommand(const std::vector<std::string> &commands,
                                 std::size_t clientId) {
  LOG_DEBUG("Getting the value {}", commands[1]);
  auto val = getValue(commands[1]);
  if (val) {
    return Server::Reply{RESP::toBString(*val)};
  } else {
    return Server::Reply{RESP::NullBString};
  }
  return Server::Reply{"+" + commands[1] + "\r\n"};
}

Server::Reply Server::setCommand(const std::vector<std::string> &commands,
                                 std::size_t clientId) {
  if (commands.size() != 3 && commands.size() != 5) {
    return Server::Reply{RESP::NullBString};
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
  propagateToReplicas(commands);
  return Server::Reply{"+OK\r\n"};
}

Server::Reply Server::configCommand(const std::vector<std::string> &commands,
                                    std::size_t clientId) {
  if (commands[1] == "GET" || commands[1] == "get") {
    auto value = config_.getField(commands[2]);
    std::string valueStr = value.to_string();
    return Server::Reply{RESP::toStringArray({commands[2], valueStr})};
  }
  return Server::Reply{RESP::NullBString};
}

Server::Reply Server::keysCommand(const std::vector<std::string> &commands,
                                  std::size_t clientId) {
  if (commands.size() != 2) {
    return Server::Reply{RESP::NullBString};
  }
  std::string pattern = commands[1];
  if (size_t loc = pattern.find('*'); loc != std::string::npos) {
    pattern.insert(loc, ".");
  }
  LOG_DEBUG("Searching using the pattern {}", pattern);
  const std::regex regex(pattern);
  std::vector<std::string> matchedKeys;
  for (const auto &record : data_) {
    if (std::regex_match(record.first, regex)) {
      matchedKeys.push_back(record.first);
    }
  }
  LOG_DEBUG("Matched KEYS {}", matchedKeys);
  return Server::Reply{RESP::toStringArray(matchedKeys)};
}

Server::Reply Server::infoCommand(const std::vector<std::string> &commands,
                                  std::size_t clientId) {
  std::vector<std::string> info;
  if (isReplica()) {
    info.push_back("role:slave");
  } else {
    info.push_back("role:master");
  }
  info.push_back("master_replid:" + masterReplId);
  info.push_back("master_repl_offset:" + std::to_string(masterReplOffset));
  info.push_back("");
  std::string infoBString = RESP::toStringArray(info);

  if (commands.size() == 2 && commands[1] == "replication") {
    // Return replication info only
    return Server::Reply{infoBString};
  }

  return Server::Reply{
      infoBString}; // return all available options, more stuff in the future.
}

Server::Reply Server::replconfCommand(const std::vector<std::string> &commands,
                                      std::size_t clientId) {
  if (commands.size() != 3) {
    return Server::Reply{RESP::NullBString};
  }
  return Server::Reply{"+OK\r\n"};
}

Server::Reply Server::psyncCommand(const std::vector<std::string> &commands,
                                   std::size_t clientId) {
  if (commands.size() != 3) {
    return Server::Reply{RESP::NullBString};
  }
  Server::Reply reply;
  reply.push_back("+FULLRESYNC " + masterReplId + " " +
                  std::to_string(masterReplOffset) + "\r\n");
  reply.push_back("$0\r\n");
  LOG_INFO("Marking client {} as a replica", clientId);
  if (clientId > 0 && clientId < clients.size()) {
    replicas.push_back(clients[clientId]);
  } else {
    LOG_ERROR("Replica is requesting SYNC but no client id is registered.");
  }

  return reply;
}

std::optional<Server::Reply> Server::handleRequest(const std::string &message,
                                                   std::size_t clientId) {
  if (message.empty()) {
    LOG_DEBUG("Received an empty message");
    return Server::Reply{"\r\n"};
  }
  if (message[0] != RESP::DataType::ARRAY) {
    LOG_DEBUG("Received a non array command {}", message);
    return Server::Reply{"\r\n"};
  }
  std::optional<std::vector<std::string>> commands = RESP::parseArray(message);
  if (!commands) {
    LOG_DEBUG("Received a non array command {}", message);
    return std::nullopt;
  }
  return handleCommands(*commands, clientId);
}

} // namespace Redis