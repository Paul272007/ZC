#include <gtest/gtest.h>

#include "pkgs/LocalTarget.h"

TEST(LocalTargetTest, ParseEmpty)
{
  std::vector<std::string> empty_strs;
  auto                     targets = zc::LocalTarget::parse(empty_strs);
  EXPECT_TRUE(targets.empty());
}

TEST(LocalTargetTest, ParseWithoutVersion)
{
  std::vector<std::string> strs    = { "pkgA", "pkgB" };
  auto                     targets = zc::LocalTarget::parse(strs);

  ASSERT_EQ(targets.size(), 2);

  EXPECT_EQ(targets[0].name, "pkgA");
  EXPECT_TRUE(targets[0].version.is_empty());

  EXPECT_EQ(targets[1].name, "pkgB");
  EXPECT_TRUE(targets[1].version.is_empty());
}

TEST(LocalTargetTest, ParseWithVersion)
{
  std::vector<std::string> strs    = { "pkgA@1.0.0", "pkgB@latest" };
  auto                     targets = zc::LocalTarget::parse(strs);

  ASSERT_EQ(targets.size(), 2);

  EXPECT_EQ(targets[0].name, "pkgA");
  EXPECT_EQ(targets[0].version.string(), "1.0.0");

  EXPECT_EQ(targets[1].name, "pkgB");
  EXPECT_TRUE(targets[1].version.is_latest());
}
