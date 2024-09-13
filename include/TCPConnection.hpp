#ifndef __TCP_CONNECTION_HPP__
#define __TCP_CONNECTION_HPP__
#include "Logging.hpp"
#include "RedisServer.hpp"
#include <asio.hpp>
#include <asio/post.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
using asio::ip::tcp;
class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
  using ptr = std::shared_ptr<TCPConnection>;
  TCPConnection(asio::io_context &io_context, Redis::Server::SharedPtr redisPtr)
      : rServer(redisPtr), ioContext(io_context), socket_(ioContext) {}

  static ptr create(asio::io_context &io_context,
                    Redis::Server::SharedPtr redisPtr) {
    return std::make_shared<TCPConnection>(io_context, redisPtr);
  }
  tcp::socket &socket() { return socket_; }
  void start() {
    asio::async_read_until(socket_, recvMsg_, "\r\n",
                           std::bind(&TCPConnection::handle_new_message,
                                     shared_from_this(),
                                     std::placeholders::_1));
  }

private:
  void handle_new_message(const std::error_code &error) {
    std::string messageStr =
        std::string((std::istreambuf_iterator<char>(&recvMsg_)),
                    std::istreambuf_iterator<char>());
    LOG_INFO("Received a new message {}", messageStr);
    // TODO check how to discard empty messages and call the start function
    // again.
    if (messageStr.empty()) {
      asio::post(ioContext,
                 std::bind(&TCPConnection::start, shared_from_this()));
      return;
    }
    std::optional<std::string> message =
        rServer->handleRequest(messageStr); //"+PONG\r\n";
    if (!message) {
      LOG_DEBUG("no message to send. ");
      asio::post(ioContext,
                 std::bind(&TCPConnection::start, shared_from_this()));
      return;
    }
    LOG_INFO("Sending REPLY {}", *message);
    std::string messageAsString = *message;
    asio::async_write(socket_, asio::buffer(messageAsString),
                      std::bind(&TCPConnection::handle_write,
                                shared_from_this(), std::placeholders::_1,
                                std::placeholders::_2));
    LOG_INFO("PONG sent");
  }
  void handle_write(const asio::error_code &ec, size_t) {
    if (!ec) {
      start();
    }
  }
  Redis::Server::SharedPtr rServer;
  asio::io_context &ioContext;
  tcp::socket socket_;
  asio::streambuf recvMsg_;
};
#endif