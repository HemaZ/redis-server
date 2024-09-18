#include "RDBFile.hpp"
#include "Logging.hpp"
#include <array>
#include <quill/std/Array.h>
#include <unordered_map>
#include <vector>
namespace Redis {

void parseHeader(std::ifstream &fs) {
  u_char bytes[9];
  fs.read(reinterpret_cast<char *>(bytes), 9);
}

std::optional<Database> parseDatabase(std::ifstream &fs) {
  u_char bytes[4];
  u_char byte;
  fs.read(reinterpret_cast<char *>(bytes), 4); // first 4 bytes
  Database data;
  while (fs.read(reinterpret_cast<char *>(&byte), 1)) {
    if (byte == 0x00) {
      u_char length;
      // Read the Key
      fs.read(reinterpret_cast<char *>(&length), 1);
      std::vector<u_char> key(static_cast<int>(length));
      fs.read(reinterpret_cast<char *>(key.data()), static_cast<int>(length));
      std::string keyStr(key.begin(), key.end());

      // Read the value
      fs.read(reinterpret_cast<char *>(&length), 1);
      std::vector<u_char> value(static_cast<int>(length));
      fs.read(reinterpret_cast<char *>(value.data()), static_cast<int>(length));
      std::string valStr(value.begin(), value.end());

      data[keyStr] = {valStr};
    }
  }
  return data;
}

std::optional<Database> parseRDBFile(const std::string &filePath) {
  std::ifstream fs(filePath, std::ios::binary);
  if (!fs.is_open()) {
    return std::nullopt;
  }
  parseHeader(fs);
  u_char byte;
  while (fs.read(reinterpret_cast<char *>(&byte), 1)) {
    if (byte == OpCodes::METADATA) {
      // TODO parse it
    }
    if (byte == OpCodes::SELECTDB) {
      return parseDatabase(fs);
    }
  }
  return std::nullopt;
}
} // namespace Redis
