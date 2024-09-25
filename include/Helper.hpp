#ifndef __REDIS_SERVER_HELPER_HPP__
#define __REDIS_SERVER_HELPER_HPP__
#include <algorithm>
#include <cctype>
#include <ctime>
#include <random>
#include <string>

/**
 * @brief Convert a string to lowercase.
 *
 * This function takes a string and converts all its characters to lowercase.
 * It uses the std::transform algorithm with a lambda function that applies
 * std::tolower to each character.
 *
 * @param s The input string to be converted.
 * @return std::string The lowercase version of the input string.
 */
inline std::string strTolower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); } // correct
  );
  return s;
}

/**
 * @brief Generate a random string of a given length
 *
 * @param length The length of the string to generate
 * @return std::string A randomly generated string of the specified length
 *                     containing alphanumeric characters (0-9, A-Z, a-z)
 */
inline std::string randomString(std::size_t length) {
  std::srand(std::time(0)); // use current time as seed for random generator
  auto randChar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randChar);
  return str;
}
#endif