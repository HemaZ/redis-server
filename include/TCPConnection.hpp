#ifndef __TCP_CONNECTION_HPP__
#define __TCP_CONNECTION_HPP__
#include "Logging.hpp"
#include "RedisServer.hpp"
#include <asio.hpp>
#include <asio/post.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <string>
using asio::ip::tcp;
class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
  using SharedPtr = std::shared_ptr<TCPConnection>;
  TCPConnection(asio::io_context &io_context, Redis::Server::SharedPtr redisPtr)
      : rServer(redisPtr), ioContext(io_context), socket_(ioContext) {}

  /**
   * @brief Create a new TCPConnection giving the asio context and
   * the Redis server.
   *
   * @param io_context asio context.
   * @param redisPtr Redis server.
   * @return SharedPtr Ptr to the created TCPConnection.
   */
  static SharedPtr create(asio::io_context &io_context,
                          Redis::Server::SharedPtr redisPtr) {
    auto ptr = std::make_shared<TCPConnection>(io_context, redisPtr);
    ptr->setClientId(redisPtr->registerClient(ptr->weak_from_this()));
    return ptr;
  }

  /**
   * @brief Get the TCP socket.
   *
   * @return tcp::socket& reference to the underlying tcp socket.
   */
  tcp::socket &socket() { return socket_; }

  /**
   * @brief Start reading messages from the socket.
   *
   */
  void start() {
    asio::async_read_until(socket_, recvMsg_, "\r\n",
                           std::bind(&TCPConnection::handle_new_message,
                                     shared_from_this(),
                                     std::placeholders::_1));
  }
  void setClientId(std::size_t id) { clientId = id; }

  void send_message(const std::string &msg) {
    LOG_INFO("Sending message: {}", msg);
    asio::async_write(socket_, asio::buffer(msg),
                      [](const asio::error_code &ec, size_t) {});
  }

private:
  std::size_t clientId = 0;
  void handle_new_message(const std::error_code &error) {
    std::string messageStr =
        std::string((std::istreambuf_iterator<char>(&recvMsg_)),
                    std::istreambuf_iterator<char>());
    LOG_INFO("Received a new message {}", messageStr);
    if (messageStr.empty()) {
      asio::post(ioContext,
                 std::bind(&TCPConnection::start, shared_from_this()));
      return;
    }
    std::optional<Redis::Server::Reply> message =
        rServer->handleRequest(messageStr, clientId); //"+PONG\r\n";
    if (!message) {
      LOG_DEBUG("no message to send. ");
      asio::post(ioContext,
                 std::bind(&TCPConnection::start, shared_from_this()));
      return;
    }
    LOG_INFO("Sending REPLY with {} messages ", message->size());
    for (const auto &msg : *message) {
      msgsQ_.push(msg);
    }

    // send_new_msg(asio::error_code(0, std::generic_category()), 0);
    asio::post(ioContext,
               std::bind(&TCPConnection::send_new_msg, shared_from_this(),
                         asio::error_code(0, std::generic_category()), 0));
  }

  void send_new_msg(const asio::error_code &ec, size_t) {
    if (msgsQ_.empty() && !ec) {
      start();
    } else {
      std::string messageAsString = msgsQ_.front();
      msgsQ_.pop();
      LOG_INFO("Sending message: {}", messageAsString);
      asio::async_write(socket_, asio::buffer(messageAsString),
                        std::bind(&TCPConnection::send_new_msg,
                                  shared_from_this(), std::placeholders::_1,
                                  std::placeholders::_2));
    }
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
  std::queue<std::string> msgsQ_;
};
#endif