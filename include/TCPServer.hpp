#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__
#include "Logging.hpp"
#include "RedisServer.hpp"
#include "TCPConnection.hpp"
#include <asio.hpp>

using asio::ip::tcp;
class TCPServer {
public:
  /**
   * @brief Construct a new TCPServer.
   *
   * @param io asio io context.
   * @param port Port number.
   */
  explicit TCPServer(asio::io_context &io, int port,
                     Redis::Server::SharedPtr redisServer)
      : ioContext_(io), acceptor_(ioContext_, tcp::endpoint(tcp::v4(), port)),
        redisPtr(redisServer) {}

  /**
   * @brief Start listening on the port and accept new connections.
   *
   */
  void start() {
    LOG_INFO("Waiting for client to connect");
    TCPConnection::SharedPtr newClient =
        TCPConnection::create(ioContext_, redisPtr);
    acceptor_.async_accept(newClient->socket(),
                           [this, newClient](std::error_code error) {
                             if (!error) {
                               LOG_INFO("A client connected successfully");
                               newClient->start();
                             }
                             start();
                           });
  }

private:
  asio::io_context &ioContext_;
  tcp::acceptor acceptor_;
  Redis::Server::SharedPtr redisPtr;
};
#endif