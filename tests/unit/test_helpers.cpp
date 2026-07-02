#include <gtest/gtest.h>

#include "helpers.h"

TEST(HelpersTest, EscCommand)
{
  // Escaping normal string
  EXPECT_EQ(zc::esc("hello"), "'hello'");
  // Escaping string with spaces
  EXPECT_EQ(zc::esc("hello world"), "'hello world'");
  // Escaping string with quotes
  EXPECT_EQ(zc::esc("hello \"world\""), "'hello \"world\"'");
  // Escaping string with single quotes
  EXPECT_EQ(zc::esc("hello 'world'"), "'hello '\\''world'\\'''");
}

TEST(HelpersTest, JoinCommand)
{
  std::vector<std::string> vec = { "a", "b", "c" };
  EXPECT_EQ(zc::join(vec, "-"), "a-b-c");

  std::vector<std::string> empty;
  EXPECT_EQ(zc::join(empty, "-"), "");
}

TEST(HelpersTest, UpperCommand)
{
  EXPECT_EQ(zc::upper("hello"), "HELLO");
  EXPECT_EQ(zc::upper("Hello World 123"), "HELLO WORLD 123");
  EXPECT_EQ(zc::upper(""), "");
}

TEST(HelpersTest, LowerCommand)
{
  EXPECT_EQ(zc::lower("HELLO"), "hello");
  EXPECT_EQ(zc::lower("Hello World 123"), "hello world 123");
  EXPECT_EQ(zc::lower(""), "");
}

TEST(HelpersTest, SplitCommand)
{
  std::vector<std::string> expected1 = { "a", "b", "c" };
  EXPECT_EQ(zc::split("a,b,c", ','), expected1);

  std::vector<std::string> expected2 = { "hello", "world" };
  EXPECT_EQ(zc::split("hello world", ' '), expected2);

  std::vector<std::string> expected_empty = {};
  EXPECT_EQ(zc::split("", ','), expected_empty);
}

TEST(HelpersTest, MergeCommand)
{
  std::vector<std::string> src  = { "a", "b", "c", "d" };
  std::vector<std::string> dest = { "x", "a", "y" };

  zc::merge(src, dest);

  std::vector<std::string> expected = { "x", "a", "y", "b", "c", "d" };
  EXPECT_EQ(dest, expected);
}

TEST(HelpersTest, CheckNameCommand)
{
  EXPECT_NO_THROW(zc::check_name("valid_name"));
  EXPECT_NO_THROW(zc::check_name("valid-name"));

  EXPECT_THROW(zc::check_name("-invalid"), zc::ZCException);
  EXPECT_THROW(zc::check_name("Makefile"), zc::ZCException);
  EXPECT_THROW(zc::check_name("invalid@name"), zc::ZCException);
  EXPECT_THROW(zc::check_name("invalid name"), zc::ZCException);
}

TEST(HelpersTest, Base64EncodeCommand)
{
  EXPECT_EQ(zc::base64_encode("hello world"), "aGVsbG8gd29ybGQ=");
  EXPECT_EQ(zc::base64_encode(""), "");
  EXPECT_EQ(zc::base64_encode("A"), "QQ==");
}

TEST(HelpersTest, StrToPath)
{
  std::vector<std::string>           str_vec  = { "path/a", "path/b" };
  std::vector<std::filesystem::path> path_vec = zc::str_to_path(str_vec);

  ASSERT_EQ(path_vec.size(), 2);
  EXPECT_EQ(path_vec[0], std::filesystem::path("path/a"));
  EXPECT_EQ(path_vec[1], std::filesystem::path("path/b"));
}
