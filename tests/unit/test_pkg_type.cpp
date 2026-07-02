#include <gtest/gtest.h>

#include "pkgs/PkgType.h"

TEST(PkgTypeTest, ToString)
{
  EXPECT_EQ(zc::pkg_type_to_str(zc::PkgType::BIN), "BIN");
  EXPECT_EQ(zc::pkg_type_to_str(zc::PkgType::LIB), "LIB");
  EXPECT_EQ(zc::pkg_type_to_str(zc::PkgType::HEADER), "HEADER");
  EXPECT_EQ(zc::pkg_type_to_str(zc::PkgType::COMPOSE), "COMPOSE");
  EXPECT_EQ(zc::pkg_type_to_str(zc::PkgType::UNDEF), "UNDEF");
}

TEST(PkgTypeTest, ToPrettyString)
{
  EXPECT_EQ(zc::pkg_type_to_pretty_str(zc::PkgType::BIN), "Binary");
  EXPECT_EQ(zc::pkg_type_to_pretty_str(zc::PkgType::LIB), "Library");
  EXPECT_EQ(zc::pkg_type_to_pretty_str(zc::PkgType::HEADER), "Header-only library");
  EXPECT_EQ(zc::pkg_type_to_pretty_str(zc::PkgType::COMPOSE), "Composed package");
  EXPECT_EQ(zc::pkg_type_to_pretty_str(zc::PkgType::UNDEF), "Undefined");
}

TEST(PkgTypeTest, FromString)
{
  EXPECT_EQ(zc::pkg_type_from_str("BIN"), zc::PkgType::BIN);
  EXPECT_EQ(zc::pkg_type_from_str("bin"), zc::PkgType::BIN);
  EXPECT_EQ(zc::pkg_type_from_str("LIB"), zc::PkgType::LIB);
  EXPECT_EQ(zc::pkg_type_from_str("HEADER"), zc::PkgType::HEADER);
  EXPECT_EQ(zc::pkg_type_from_str("COMPOSE"), zc::PkgType::COMPOSE);
  EXPECT_EQ(zc::pkg_type_from_str("UNKNOWN"), zc::PkgType::UNDEF);
}
