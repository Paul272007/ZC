#include "framework/Version.hpp"

#include <gtest/gtest.h>

namespace zc::tests
{

TEST(VersionTest, DefaultAndExplicitConstruction)
{
  Version v1;
  EXPECT_EQ(v1.major(), 0);
  EXPECT_EQ(v1.minor(), 0);
  EXPECT_EQ(v1.patch(), 0);
  EXPECT_TRUE(v1.isEmpty());

  Version v2(1, 2, 3);
  EXPECT_EQ(v2.major(), 1);
  EXPECT_EQ(v2.minor(), 2);
  EXPECT_EQ(v2.patch(), 3);
  EXPECT_EQ(v2.string(), "1.2.3");
}

TEST(VersionTest, StringParsing)
{
  Version v1("1.2.3");
  EXPECT_EQ(v1.major(), 1);
  EXPECT_EQ(v1.minor(), 2);
  EXPECT_EQ(v1.patch(), 3);

  Version v2("latest");
  EXPECT_TRUE(v2.isLatest());

  Version v4("");
  EXPECT_TRUE(v4.isEmpty());

  EXPECT_THROW(Version("invalid.version.str"), Exception);
}

TEST(VersionTest, Comparison)
{
  Version v1(1, 0, 0);
  Version v2(1, 2, 0);
  Version v3(1, 2, 1);
  Version v4(2, 0, 0);

  EXPECT_LT(v1, v2);
  EXPECT_LT(v2, v3);
  EXPECT_LT(v3, v4);
  EXPECT_EQ(v1, Version(1, 0, 0));
  EXPECT_NE(v1, v2);
}

TEST(VersionTest, StreamAndFormat)
{
  Version            v(2, 4, 1);
  std::ostringstream oss;
  oss << v;
  EXPECT_EQ(oss.str(), "2.4.1");

  std::string formatted = std::format("{}", v);
  EXPECT_EQ(formatted, "2.4.1");
}

TEST(VersionTest, JsonSerialization)
{
  Version        v(3, 1, 4);
  nlohmann::json j = v;
  EXPECT_EQ(j, "3.1.4");

  Version v_deserialized = j.get<Version>();
  EXPECT_EQ(v_deserialized, v);
}

} // namespace zc::tests
