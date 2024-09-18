#ifndef __REDIS_RDB_FILE_HPP__
#define __REDIS_RDB_FILE_HPP__
#include "Types.hpp"
#include <fstream>
#include <iostream>
#include <string>
namespace Redis {

/**
 * @brief RDB Files Op codes
 * https://rdb.fnordig.de/file_format.html#op-codes
 *
 */
enum OpCodes {
  METADATA = 0xFA,
  EORDBF = 0xFF,
  SELECTDB = 0xFE,
  EXPIRETIME = 0xFD,
  EXPIRETIMEMS = 0xFC,
  RESIZEDB = 0xFB,
  AUX = 0xFA,
};

/**
 * @brief Parse a RDB file and return the stored database.
 *
 * @param filePath Absolute path to the rdb file.
 * @return std::optional<Database> None if error opening or parsing the file.
 */
std::optional<Database> parseRDBFile(const std::string &filePath);
} // namespace Redis
#endif