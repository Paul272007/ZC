#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "config/PConf.h"
#include "excepts/ZCException.h"
#include "helpers.h"

class PConfTest : public ::testing::Test
{
protected:
  std::filesystem::path temp_file = "test_zc.json";

  void SetUp() override
  {
    // Prepare a valid JSON configuration
    nlohmann::json j = {
      { "name", "my_project" },
      { "author", "Me" },
      { "target", "my_target" },
      { "src_dirs", { "source" } },
      { "include_dirs", { "include" } },
      { "type", "BIN" },
      { "version", "1.0.0" },
      { "macros", { { "DEBUG", "1" } } },
      { "dependencies",
        { { "libA", { { "origin", "main" }, { "version", "1.2.3" }, { "static", false } } } } },
      { "languages",
        { { "CXX", { { "compiler", "g++" }, { "std", "c++20" }, { "flags", { "-Wall" } } } } } }
    };
    zc::write_json(j, temp_file);
  }

  void TearDown() override
  {
    if (std::filesystem::exists(temp_file))
      std::filesystem::remove(temp_file);
  }
};

TEST_F(PConfTest, LoadValidConfig)
{
  zc::PConf conf(temp_file);

  EXPECT_EQ(conf.name, "my_project");
  EXPECT_EQ(conf.author, "Me");
  EXPECT_EQ(conf.target, "my_target");
  EXPECT_EQ(conf.src_dirs.size(), 1);
  EXPECT_EQ(conf.src_dirs[0], "source");
  EXPECT_EQ(conf.include_dirs.size(), 1);
  EXPECT_EQ(conf.include_dirs[0], "include");
  EXPECT_EQ(conf.type, zc::PkgType::BIN);
  EXPECT_EQ(conf.version.string(), "1.0.0");

  ASSERT_TRUE(conf.macros.contains("DEBUG"));
  EXPECT_EQ(conf.macros["DEBUG"], "1");

  ASSERT_TRUE(conf.dependencies.contains("libA"));
  EXPECT_EQ(conf.dependencies["libA"].version.string(), "1.2.3");

  ASSERT_TRUE(conf.languages.contains(zc::Language::CXX));
  EXPECT_EQ(conf.languages[zc::Language::CXX].compiler, "g++");
}

TEST_F(PConfTest, ModifyAndWriteConfig)
{
  {
    zc::PConf      conf(temp_file);
    zc::Dependency dep;
    dep.name    = "libB";
    dep.version = zc::Version("2.0.0");
    conf.add_dependency(dep);

    conf.name = "new_name";
    // Destructor will write changes to file
  }

  // Reload
  zc::PConf new_conf(temp_file);
  EXPECT_EQ(new_conf.name, "new_name");
  ASSERT_TRUE(new_conf.dependencies.contains("libA"));
  ASSERT_TRUE(new_conf.dependencies.contains("libB"));
  EXPECT_EQ(new_conf.dependencies["libB"].version.string(), "2.0.0");
}

TEST_F(PConfTest, RemoveDependency)
{
  {
    zc::PConf conf(temp_file);
    conf.remove_dependency("libA");
  }

  zc::PConf new_conf(temp_file);
  EXPECT_FALSE(new_conf.dependencies.contains("libA"));
}

TEST_F(PConfTest, ChangeDependencyVersion)
{
  {
    zc::PConf conf(temp_file);
    conf.change_dependency_version("libA", zc::Version("2.0.0"));
  }

  zc::PConf new_conf(temp_file);
  ASSERT_TRUE(new_conf.dependencies.contains("libA"));
  EXPECT_EQ(new_conf.dependencies["libA"].version.string(), "2.0.0");
}

TEST_F(PConfTest, InvalidExceptions)
{
  zc::PConf conf(temp_file);

  zc::Dependency d;
  d.name = "libA"; // Already exists
  EXPECT_THROW(conf.add_dependency(d), zc::ZCException);

  EXPECT_THROW(conf.change_dependency_version("not_found", zc::Version("1.0.0")), zc::ZCException);

  EXPECT_THROW(conf.remove_dependency("not_found"), zc::ZCException);
}
