#ifndef __REDIS_SERVER_HELPER_HPP__
#define __REDIS_SERVER_HELPER_HPP__
#include <algorithm>
#include <cctype>
#include <string>
inline std::string strTolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); } // correct
  );
  return s;
}
#endif