#include <gtest/gtest.h>

#include "utils.hpp"

TEST(UtilsTest, DateTest) {
  EXPECT_EQ(parseDate("25.02.2026"), 20260225);
  EXPECT_EQ(parseDate("01.01.2000"), 20000101);
  EXPECT_EQ(parseDate("31.12.1999"), 19991231);
}

TEST(UtilsTest, TimeTest) {
  EXPECT_EQ(parseTime("00:00:00"), 0);
  EXPECT_EQ(parseTime("12:34:56"), 45296);
  EXPECT_EQ(parseTime("23:59:59"), 86399);

  EXPECT_EQ(kDaySeconds, 86400);
}