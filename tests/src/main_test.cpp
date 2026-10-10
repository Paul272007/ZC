#include <gtest/gtest.h>

TEST(ZcSanityTest, ToolchainAndGTestWork)
{
  EXPECT_EQ(1 + 1, 2);
  EXPECT_TRUE(true);
}
