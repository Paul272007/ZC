#include <gtest/gtest.h>

#include "excepts/ZCException.h"
#include "Version.h"

TEST(VersionTest, ConstructorFromString)
{
  zc::Version v1("1.2.3");
  EXPECT_EQ(v1.major(), 1);
  EXPECT_EQ(v1.minor(), 2);
  EXPECT_EQ(v1.patch(), 3);

  zc::Version v2("2.0");
  EXPECT_EQ(v2.major(), 2);
  EXPECT_EQ(v2.minor(), 0);
  EXPECT_EQ(v2.patch(), 0);

  zc::Version v3("3");
  EXPECT_EQ(v3.major(), 3);
  EXPECT_EQ(v3.minor(), 0);
  EXPECT_EQ(v3.patch(), 0);

  zc::Version v_empty("");
  EXPECT_TRUE(v_empty.is_empty());

  zc::Version v_latest("latest");
  EXPECT_TRUE(v_latest.is_latest());

  zc::Version v_default("default");
  EXPECT_TRUE(v_default.is_default());
}

TEST(VersionTest, ConstructorFromInts)
{
  zc::Version v(1, 2, 3);
  EXPECT_EQ(v.major(), 1);
  EXPECT_EQ(v.minor(), 2);
  EXPECT_EQ(v.patch(), 3);
}

TEST(VersionTest, InvalidString)
{
  EXPECT_THROW(zc::Version("1.2.a"), zc::ZCException);
  EXPECT_THROW(zc::Version("invalid"), zc::ZCException);
}

TEST(VersionTest, StringConversion)
{
  zc::Version v(1, 2, 3);
  EXPECT_EQ(v.string(), "1.2.3");
}

TEST(VersionTest, ComparisonOperators)
{
  zc::Version v1(1, 2, 3);
  zc::Version v2(1, 2, 4);
  zc::Version v3(2, 0, 0);
  zc::Version v4(1, 2, 3);

  EXPECT_LT(v1, v2);
  EXPECT_LT(v2, v3);
  EXPECT_GT(v3, v1);
  EXPECT_EQ(v1, v4);
  EXPECT_LE(v1, v4);
  EXPECT_GE(v1, v4);
  EXPECT_NE(v1, v2);
}
