#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "pkgs/Pkg.h"

TEST(PkgTest, JsonSerialization)
{
  nlohmann::json j = { { "type", "BIN" },         { "target", "my_target" },
                       { "origin", "my_origin" }, { "path", "/some/path" },
                       { "default", "1.0.0" },    { "versions", { "1.0.0", "2.0.0" } } };

  zc::Pkg p;
  zc::from_json(j, p);

  EXPECT_EQ(p.type, zc::PkgType::BIN);
  EXPECT_EQ(p.target, "my_target");
  EXPECT_EQ(p.origin, "my_origin");
  EXPECT_EQ(p.path, "/some/path");
  EXPECT_EQ(p.default_version.string(), "1.0.0");
  ASSERT_EQ(p.versions.size(), 2);
  EXPECT_EQ(p.versions[0].string(), "1.0.0");
  EXPECT_EQ(p.versions[1].string(), "2.0.0");

  nlohmann::json out;
  zc::to_json(out, p);

  EXPECT_EQ(out["type"], "BIN");
  EXPECT_EQ(out["target"], "my_target");
  EXPECT_EQ(out["origin"], "my_origin");
  EXPECT_EQ(out["path"], "/some/path");
  EXPECT_EQ(out["default"], "1.0.0");
  EXPECT_EQ(out["versions"].size(), 2);
  EXPECT_EQ(out["versions"][0], "1.0.0");
  EXPECT_EQ(out["versions"][1], "2.0.0");
}
