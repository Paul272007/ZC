#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "config/Dependency.h"

TEST(DependencyTest, JsonSerialization)
{
  nlohmann::json j = { { "origin", "custom_origin" }, { "static", true }, { "version", "1.5.0" } };

  zc::Dependency dep;
  // name is not serialized/deserialized in Dependency's from_json
  zc::from_json(j, dep);

  EXPECT_EQ(dep.origin, "custom_origin");
  EXPECT_TRUE(dep.static_link);
  EXPECT_EQ(dep.version.string(), "1.5.0");

  nlohmann::json out;
  zc::to_json(out, dep);

  EXPECT_EQ(out["origin"], "custom_origin");
  EXPECT_TRUE(out["static"]);
  EXPECT_EQ(out["version"], "1.5.0");
}

TEST(DependencyTest, JsonSerializationDefault)
{
  nlohmann::json j = { { "origin", "main" }, { "version", "latest" } };

  zc::Dependency dep;
  dep.static_link = false;

  zc::from_json(j, dep);

  EXPECT_EQ(dep.origin, "main");
  EXPECT_FALSE(dep.static_link);
  EXPECT_TRUE(dep.version.is_latest());
}
