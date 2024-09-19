#ifndef __REDIS_SERVER_TYPES_HPP_
#define __REDIS_SERVER_TYPES_HPP_
#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
namespace Redis {
/**
 * @brief Database record which contains the value as string,
 * and an Optional expiry date.
 *
 */
struct Record {
  std::string data;
  std::optional<std::chrono::time_point<std::chrono::system_clock>> expiry;
  /**
   * @brief Set the Expiry giving a period in milliseconds.
   *
   * @param milliseconds expiry period in milliseconds.
   */
  void setExpiry(int milliseconds) {
    expiry = std::chrono::system_clock::now() +
             std::chrono::milliseconds(milliseconds);
  }

  /**
   * @brief Set the Expiry giving a unix timestamp in milliseconds.
   *
   * @param unixTsMilliSeconds miliseconds since epoch.
   */
  void setExpiry(unsigned long unixTsMilliSeconds) {
    expiry = std::chrono::system_clock::time_point{
        std::chrono::milliseconds{unixTsMilliSeconds}};
  }

  /**
   * @brief Set the Expiry giving a unix timestamp in seconds.
   *
   * @param unixTsSeconds seconds since epoch.
   */
  void setExpiry(unsigned int unixTsSeconds) {
    expiry = std::chrono::system_clock::time_point{
        std::chrono::seconds{unixTsSeconds}};
  }

  /**
   * @brief Return true if the record is already expired.
   */
  bool expired() const {
    if (expiry == std::nullopt) {
      return false;
    }
    return *expiry <= std::chrono::system_clock::now();
  }
};

/**
 * @brief Database is defined as an unordered_map with string keys
 * and @sa Record values.
 *
 */
using Database = std::unordered_map<std::string, Record>;
} // namespace Redis

#endif