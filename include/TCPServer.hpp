#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__
#include "Logging.hpp"
#include "TCPConnection.hpp"
#include <asio.hpp>

using asio::ip::tcp;
class TCPServer {
public:
  explicit TCPServer(asio::io_context &io, int port = 6379)
      : ioContext_(io), acceptor_(ioContext_, tcp::endpoint(tcp::v4(), port)) {}

  void start() {
    LOG_INFO("Waiting for client to connect");
    TCPConnection::ptr newClient = TCPConnection::create(ioContext_);
    acceptor_.async_accept(newClient->socket(),
                           std::bind(&TCPServer::handleAccept, this, newClient,
                                     std::placeholders::_1));
  }

private:
  void handleAccept(TCPConnection::ptr client, const std::error_code &error) {
    if (!error) {
      LOG_INFO("A client connected sucessfully");
      client->start();
    }
    start();
  }
  asio::io_context &ioContext_;
  tcp::acceptor acceptor_;
};
#endif