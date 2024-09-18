#ifndef __REDIS_SERVER_CONFIG_HPP__
#define __REDIS_SERVER_CONFIG_HPP__
#include <rttr/registration>
#include <string>
using namespace rttr;

namespace Redis {
/**
 * @brief Redis Server config.
 *
 */
struct Config {
  std::string dir = "/tmp/redis-data";
  std::string dbfilename = "dump.rdb";

  rttr::variant getField(const std::string &fieldName) {
    property prop = type::get(*this).get_property(fieldName);
    variant varProp = prop.get_value(*this);
    return varProp;
  }
};

RTTR_REGISTRATION {
  registration::class_<Config>("Config")
      .constructor<>()
      .property("dir", &Config::dir)
      .property("dbfilename", &Config::dbfilename);
}
} // namespace Redis

#endif