#ifndef __TCP_CONNECTION_HPP__
#define __TCP_CONNECTION_HPP__
#include "quill_wrapper/overwrite_macros.h"
#include "quill_wrapper/quill_wrapper.h"
#include <asio.hpp>
#include <functional>
#include <memory>
#include <string>

using asio::ip::tcp;
class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
  using ptr = std::shared_ptr<TCPConnection>;
  TCPConnection(asio::io_context &io_context) : socket_(io_context) {}
  static ptr create(asio::io_context &io_context) {
    return std::make_shared<TCPConnection>(io_context);
  }
  tcp::socket &socket() { return socket_; }
  void start() {
    asio::async_read_until(socket_, recvMsg, "\r\n",
                           std::bind(&TCPConnection::handle_new_message,
                                     shared_from_this(),
                                     std::placeholders::_1));
  }

private:
  void handle_new_message(const std::error_code &error) {
    std::string messageStr =
        std::string((std::istreambuf_iterator<char>(&recvMsg)),
                    std::istreambuf_iterator<char>());
    LOG_INFO("Received a new message {}", messageStr);
    std::string message = "+PONG\r\n";
    LOG_INFO("Sending PONG");
    asio::async_write(socket_, asio::buffer(message),
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
  tcp::socket socket_;
  asio::streambuf recvMsg;
};
#endif