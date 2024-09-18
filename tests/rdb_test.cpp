#include "RDBFile.hpp"
#include <gtest/gtest.h>

TEST(RDB_FILE, BasicRead) {
  auto databse = Redis::parseRDBFile(RDB_FILE_PATH);
  ASSERT_TRUE(databse.has_value());
  EXPECT_EQ(databse->size(), 2);
  EXPECT_NE(databse->find("foo"), databse->end());
  EXPECT_NE(databse->find("hema"), databse->end());
}
