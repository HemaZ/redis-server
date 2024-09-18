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
  void setExpiry(int milliseconds) {
    expiry = std::chrono::system_clock::now() +
             std::chrono::milliseconds(milliseconds);
  }
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