#include <gtest/gtest.h>

#include "pkgs/LocalTarget.h"

TEST(LocalTargetTest, ParseEmpty)
{
  std::vector<std::string> empty_strs;
  auto                     parsed = zc::parse_targets(empty_strs);
  std::vector<zc::LocalTarget> targets;
  for (const auto& t : parsed) targets.push_back(zc::LocalTarget::get_target(t));
  EXPECT_TRUE(targets.empty());
}

TEST(LocalTargetTest, ParseWithoutVersion)
{
  std::vector<std::string> strs    = { "pkgA", "pkgB" };
  auto                     parsed = zc::parse_targets(strs);
  std::vector<zc::LocalTarget> targets;
  for (const auto& t : parsed) targets.push_back(zc::LocalTarget::get_target(t));

  ASSERT_EQ(targets.size(), 2);

  EXPECT_EQ(targets[0].name, "pkgA");
  EXPECT_EQ(targets[0].version.string(), "1.0.0");

  EXPECT_EQ(targets[1].name, "pkgB");
  EXPECT_EQ(targets[1].version.string(), "2.0.0");
}

TEST(LocalTargetTest, ParseWithVersion)
{
  std::vector<std::string> strs    = { "pkgA@1.0.0", "pkgB@latest" };
  auto                     parsed = zc::parse_targets(strs);
  std::vector<zc::LocalTarget> targets;
  for (const auto& t : parsed) targets.push_back(zc::LocalTarget::get_target(t));

  ASSERT_EQ(targets.size(), 2);

  EXPECT_EQ(targets[0].name, "pkgA");
  EXPECT_EQ(targets[0].version.string(), "1.0.0");

  EXPECT_EQ(targets[1].name, "pkgB");
  EXPECT_EQ(targets[1].version.string(), "2.0.0");
}
