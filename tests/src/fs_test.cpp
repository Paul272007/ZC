#include "framework/fs.hpp"

#include <gtest/gtest.h>

#include "framework/Exception.hpp"

namespace zc::tests
{

class FSTest : public ::testing::Test
{
protected:
  stdfs::path test_dir;

  void SetUp() override
  {
    test_dir = stdfs::temp_directory_path() / "zc_fs_test_dir";
    stdfs::create_directories(test_dir);
  }

  void TearDown() override
  {
    std::error_code ec;
    stdfs::remove_all(test_dir, ec);
  }
};

TEST_F(FSTest, ReadAndWriteFile)
{
  stdfs::path file_path = test_dir / "sample.txt";
  EXPECT_FALSE(fs::file_exists(file_path));

  auto write_res = fs::write(file_path, "Hello, ZC framework!");
  EXPECT_TRUE(write_res.has_value());
  EXPECT_TRUE(fs::file_exists(file_path));

  auto read_res = fs::read(file_path);
  ASSERT_TRUE(read_res.has_value());
  EXPECT_EQ(*read_res, "Hello, ZC framework!");

  fs::remove(file_path);
  EXPECT_FALSE(fs::file_exists(file_path));
}

TEST_F(FSTest, ReadNonExistentReturnsError)
{
  stdfs::path non_existent = test_dir / "does_not_exist.txt";
  auto        read_res     = fs::read(non_existent);
  EXPECT_FALSE(read_res.has_value());
}

TEST_F(FSTest, JsonReadAndWrite)
{
  stdfs::path json_path = test_dir / "data.json";
  json        original  = { { "name", "zc" }, { "version", 1 }, { "flags", { "debug", "opt" } } };

  auto write_res = fs::write_json(json_path, original);
  EXPECT_TRUE(write_res.has_value());
  EXPECT_TRUE(fs::file_exists(json_path));

  auto read_res = fs::read_json(json_path);
  ASSERT_TRUE(read_res.has_value());
  EXPECT_EQ((*read_res)["name"], "zc");
  EXPECT_EQ((*read_res)["version"], 1);
  EXPECT_EQ((*read_res)["flags"].size(), 2);

  fs::remove(json_path);
  EXPECT_FALSE(fs::file_exists(json_path));
}

TEST_F(FSTest, ReadJsonErrors)
{
  stdfs::path non_existent = test_dir / "missing.json";
  auto        missing_res  = fs::read_json(non_existent);
  EXPECT_FALSE(missing_res.has_value());

  stdfs::path bad_json = test_dir / "corrupt.json";
  (void)fs::write(bad_json, "{ this is not valid json }");
  auto corrupt_res = fs::read_json(bad_json);
  EXPECT_FALSE(corrupt_res.has_value());
}

TEST_F(FSTest, DirectoryAndFileExists)
{
  EXPECT_TRUE(fs::dir_exists(test_dir));
  EXPECT_FALSE(fs::file_exists(test_dir));

  stdfs::path dummy = test_dir / "file.bin";
  fs::write(dummy, "123");
  EXPECT_TRUE(fs::file_exists(dummy));
  EXPECT_FALSE(fs::dir_exists(dummy));
}

TEST_F(FSTest, PathFormatter)
{
  stdfs::path p         = "/foo/bar";
  std::string formatted = std::format("{}", p);
  EXPECT_EQ(formatted, "'/foo/bar'");
}

} // namespace zc::tests
