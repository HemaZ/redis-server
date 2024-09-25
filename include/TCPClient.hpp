#ifndef __REDIS_SERVER_TCP_CLIENT_HPP__
#define __REDIS_SERVER_TCP_CLIENT_HPP__
#include "Logging.hpp"
#include <asio.hpp>
#include <asio/post.hpp>

using asio::ip::tcp;

/**
 * @brief A class representing a TCP client using the Asio library.
 *
 * This class provides functionality to connect to a TCP server,
 * send messages, and receive responses.
 */
class TCPClient {
public:
  /**
   * @brief Constructs a TCPClient and establishes a connection to the server.
   *
   * @param io_context The Asio io_context to use for asynchronous operations.
   * @param ip The IP address of the server to connect to.
   * @param port The port number of the server to connect to.
   */
  TCPClient(asio::io_context &io_context, std::string ip, int port)
      : ioContext_(io_context), socket_(ioContext_) {
    tcp::resolver resolver(ioContext_);
    tcp::resolver::results_type endpoints =
        resolver.resolve(ip, std::to_string(port));
    asio::connect(socket_, endpoints);
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
    return error;
  }

private:
  /**
   * @brief Reference to the Asio io_context used for asynchronous operations.
   */
  asio::io_context &ioContext_;

  /**
   * @brief The TCP socket used for communication with the server.
   */
  tcp::socket socket_;
};

#endif