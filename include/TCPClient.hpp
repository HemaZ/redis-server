#ifndef __REDIS_SERVER_TCP_CLIENT_HPP__
#define __REDIS_SERVER_TCP_CLIENT_HPP__
#include "Logging.hpp"
#include <asio.hpp>
#include <asio/post.hpp>
#include <functional>
using asio::ip::tcp;

/**
 * @brief A class representing a TCP client using the Asio library.
 *
 * This class provides functionality to connect to a TCP server,
 * send messages, and receive responses.
 */
class TCPClient : public std::enable_shared_from_this<TCPClient> {
public:
  TCPClient(asio::io_context &io_context, std::string ip, int port)
      : ioContext_(io_context), socket_(ioContext_) {
    tcp::resolver resolver(ioContext_);
    tcp::resolver::results_type endpoints =
        resolver.resolve(ip, std::to_string(port));
    asio::connect(socket_, endpoints);
  }

  /**
   * @brief Constructs a TCPClient and establishes a connection to the server.
   *
   * @param io_context The Asio io_context to use for asynchronous operations.
   * @param ip The IP address of the server to connect to.
   * @param port The port number of the server to connect to.
   */
  static std::shared_ptr<TCPClient> create(asio::io_context &io_context,
                                           std::string ip, int port) {
    return std::make_shared<TCPClient>(io_context, ip, port);
  }

  /**
   * @brief Sends a message to the connected server.
   *
   * @param msg The message to send.
   * @return An error code indicating the success or failure of the operation.
   */
  std::error_code send(const std::string &msg) {
    asio::error_code error;
    asio::write(socket_, asio::buffer(msg), error);
    return error;
  }

  /**
   * @brief Reads a message from the connected server.
   *
   * This method reads until it encounters a "\r\n" sequence or reaches EOF.
   *
   * @param msg A reference to a string where the received message will be
   * stored.
   * @return An error code indicating the success or failure of the operation.
   */
  std::error_code read(std::string &msg) {
    asio::streambuf receive_buffer;
    asio::error_code error;
    asio::read_until(socket_, receive_buffer, "\r\n", error);
    if (error && error != asio::error::eof) {
      LOG_ERROR("Receiving failed {}", error.message());
      return error;
    }
    msg = std::string((std::istreambuf_iterator<char>(&receive_buffer)),
                      std::istreambuf_iterator<char>());
    if (callback) {
      callback(msg);
    }
    return error;
  }

  /**
   * @brief Starts asynchronous listening for incoming messages.
   *
   * This method initiates an asynchronous read operation that continuously
   * listens for incoming messages from the server. When a message is received,
   * it is processed and the callback function (if set) is called with the
   * received message. The method then recursively calls itself to continue
   * listening for more messages.
   *
   * The method uses asio's async_read_until to read until a "\r\n" sequence
   * is encountered, which is typical for Redis protocol messages.
   *
   * @note This method is non-blocking and returns immediately. The actual
   * reading and processing of messages happens asynchronously.
   */
  void listen() {
    asio::async_read_until(socket_, recvMsg_, "\r\n",
                           [this](const std::error_code &error, std::size_t) {
                             std::string messageStr = std::string(
                                 (std::istreambuf_iterator<char>(&recvMsg_)),
                                 std::istreambuf_iterator<char>());
                             LOG_INFO("Received a new message {}", messageStr);
                             if (callback) {
                               callback(messageStr);
                             }
                             listen();
                           });
  }

  /**
   * @brief Sets the callback function for handling received messages.
   *
   * This method allows the user to set a custom callback function that will be
   * called whenever a message is received from the server. The callback
   * function should take a const reference to a std::string as its parameter,
   * which will contain the received message.
   *
   * @param cb A std::function object representing the callback function to be
   * set. The function should have the signature void(const std::string&).
   */

  void setCallback(const std::function<void(const std::string &)> &cb) {
    callback = cb;
  }

private:
  std::function<void(const std::string &)> callback;
  /**
   * @brief Reference to the Asio io_context used for asynchronous operations.
   */
  asio::io_context &ioContext_;

  /**
   * @brief The TCP socket used for communication with the server.
   */
  tcp::socket socket_;
  asio::streambuf recvMsg_;
};

#endif