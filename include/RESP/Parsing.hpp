#ifndef __RESP_PARSING_HPP__
#define __RESP_PARSING_HPP__
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
namespace RESP {

/**
 * @brief RESP Datatypes defined in
 * https://redis.io/docs/latest/develop/reference/protocol-spec/
 *
 */
enum DataType {
  S_STRING = '+',
  S_ERROR = '-',
  INTEGER = ':',
  B_STRING = '$',
  ARRAY = '*',
  NULL_T = '_',
  BOOLEAN = '#',
  DOUBLE = ',',
  BIG_NUMBER = '(',
  B_ERROR = '!',
  V_STRING = '=',
  MAP = '%',
  SET = '~',
  PUSH = '>'
};

std::optional<std::string> parseBString(const std::string &command) {
  std::regex rgx("[$][0-9]+\r\n(\\w+)\r\n");
  std::smatch match;
  if (std::regex_search(command, match, rgx)) {
    return match.str(1);
  }
  return std::nullopt;
}

std::optional<std::vector<std::string>> parseArray(const std::string &command) {
  if (command.empty() || command[0] != DataType::ARRAY) {
    return std::nullopt;
  }
  std::regex pattern(R"([^\r\n]+)");
  auto words_begin =
      std::sregex_iterator(command.begin(), command.end(), pattern);
  auto words_end = std::sregex_iterator();
  std::vector<std::string> words;
  for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;
    std::string match_str = match.str();
    if (match_str.empty())
      continue;
    if (match_str[0] == DataType::B_STRING && std::next(i, 1) != words_end) {
      words.push_back(std::next(i, 1)->str());
    }
  }
  return words;
}

std::string toBString(const std::string &input) {
  return "$" + std::to_string(input.size()) + "\r\n" + input + "\r\n";
}

} // namespace RESP
#endif