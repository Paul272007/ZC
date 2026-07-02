#include <gtest/gtest.h>

#include "project/Project.h"

TEST(ProjectUtilsTest, BuildModeToString)
{
  EXPECT_EQ(zc::build_mode_to_str(zc::BuildMode::automatic), "");
  EXPECT_EQ(zc::build_mode_to_str(zc::BuildMode::release), "release");
  EXPECT_EQ(zc::build_mode_to_str(zc::BuildMode::debug), "debug");
}

TEST(ProjectUtilsTest, BuildModeFromString)
{
  EXPECT_EQ(zc::build_mode_from_str("release"), zc::BuildMode::release);
  EXPECT_EQ(zc::build_mode_from_str("RELEASE"), zc::BuildMode::release);
  EXPECT_EQ(zc::build_mode_from_str("debug"), zc::BuildMode::debug);
  EXPECT_EQ(zc::build_mode_from_str("DEBUG"), zc::BuildMode::debug);

  EXPECT_THROW(zc::build_mode_from_str("invalid"), zc::ZCException);
}
