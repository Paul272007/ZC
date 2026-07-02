#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "config/LanguageConf.h"

TEST(LanguageConfTest, JsonSerialization)
{
  nlohmann::json j = { { "compiler", "gcc" }, { "std", "c++20" }, { "flags", { "-Wall", "-O3" } } };

  zc::LanguageConf conf;
  zc::from_json(j, conf);

  EXPECT_EQ(conf.compiler, "gcc");
  EXPECT_EQ(conf.std, "c++20");
  EXPECT_EQ(conf.flags.size(), 2);
  EXPECT_EQ(conf.flags[0], "-Wall");
  EXPECT_EQ(conf.flags[1], "-O3");

  nlohmann::json out;
  zc::to_json(out, conf);

  EXPECT_EQ(out["compiler"], "gcc");
  EXPECT_EQ(out["std"], "c++20");
  EXPECT_EQ(out["flags"].size(), 2);
  EXPECT_EQ(out["flags"][0], "-Wall");
}
