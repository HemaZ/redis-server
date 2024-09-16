#ifndef __REDIS_SERVER_RESP_CONSTANTS_HPP__
#define __REDIS_SERVER_RESP_CONSTANTS_HPP__
#include <string>
namespace RESP {
constexpr auto NullBString = "$-1\r\n";
constexpr auto Null = "_\r\n";
} // namespace RESP

#endif