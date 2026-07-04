#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "pkgs/Pkg.h"

TEST(PkgTest, JsonSerialization)
{
  nlohmann::json j = { { "type", "BIN" },         { "target", "my_target" },
                       { "origin", "my_origin" }, { "path", "/some/path" },
                       { "default", "1.0.0" },    { "versions", nlohmann::json::object({ {"1.0.0", nlohmann::json::object()}, {"2.0.0", nlohmann::json::object()} }) } };

  zc::Pkg p;
  zc::from_json(j, p);

  EXPECT_EQ(p.type, zc::PkgType::BIN);
  EXPECT_EQ(p.target, "my_target");
  EXPECT_EQ(p.origin, "my_origin");
  EXPECT_EQ(p.path, "/some/path");
  EXPECT_EQ(p.default_version.string(), "1.0.0");
  ASSERT_EQ(p.versions.size(), 2);
  EXPECT_TRUE(p.versions.contains(zc::Version("1.0.0")));
  EXPECT_TRUE(p.versions.contains(zc::Version("2.0.0")));

  nlohmann::json out;
  zc::to_json(out, p);

  EXPECT_EQ(out["type"], "BIN");
  EXPECT_EQ(out["target"], "my_target");
  EXPECT_EQ(out["origin"], "my_origin");
  EXPECT_EQ(out["path"], "/some/path");
  EXPECT_EQ(out["default"], "1.0.0");
  EXPECT_EQ(out["versions"].size(), 2);
  EXPECT_TRUE(out["versions"].contains("1.0.0"));
  EXPECT_TRUE(out["versions"].contains("2.0.0"));
}
