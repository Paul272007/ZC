#include <gtest/gtest.h>

#include "framework/util/string.hpp"

namespace zc::tests
{

TEST(StringTest, Join)
{
  EXPECT_EQ(join({}, ", "), "");
  EXPECT_EQ(join({ "apple" }, ", "), "apple");
  EXPECT_EQ(join({ "apple", "banana", "cherry" }, ", "), "apple, banana, cherry");
  EXPECT_EQ(join({ "a", "b" }, ""), "ab");
}

TEST(StringTest, UpperAndLower)
{
  EXPECT_EQ(upper("hello World 123!"), "HELLO WORLD 123!");
  EXPECT_EQ(upper(""), "");
  EXPECT_EQ(lower("HELLO World 123!"), "hello world 123!");
  EXPECT_EQ(lower(""), "");
}

TEST(StringTest, Esc)
{
  EXPECT_EQ(esc("simple"), "'simple'");
  EXPECT_EQ(esc("it's"), "'it'\\''s'");
  EXPECT_EQ(esc(""), "''");
}

TEST(StringTest, Split)
{
  const std::vector<std::string> expected_abc   = { "a", "b", "c" };
  const std::vector<std::string> expected_hello = { "hello" };
  const std::vector<std::string> expected_empty = {};

  EXPECT_EQ(split("a,b,c", ','), expected_abc);
  EXPECT_EQ(split("hello", ','), expected_hello);
  EXPECT_EQ(split("", ','), expected_empty);
  EXPECT_EQ(split("a::b::c", ':'), expected_abc);
}

TEST(StringTest, Quote)
{
  EXPECT_EQ(quote("test"), "\"test\"");
  EXPECT_EQ(quote(42), "\"42\"");
  EXPECT_EQ(quote(3.14), "\"3.14\"");
}

TEST(StringTest, IEquals)
{
  EXPECT_TRUE(iequals("hello", "HELLO"));
  EXPECT_TRUE(iequals("Hello World", "hello world"));
  EXPECT_TRUE(iequals("", ""));
  EXPECT_FALSE(iequals("hello", "hell"));
  EXPECT_FALSE(iequals("hello", "world"));
}

TEST(StringTest, IsSpace)
{
  EXPECT_TRUE(is_space(' '));
  EXPECT_TRUE(is_space('\t'));
  EXPECT_TRUE(is_space('\n'));
  EXPECT_TRUE(is_space('\r'));
  EXPECT_TRUE(is_space('\v'));
  EXPECT_FALSE(is_space('a'));
  EXPECT_FALSE(is_space('0'));
}

TEST(StringTest, Trim)
{
  EXPECT_EQ(trim("  hello  "), "hello");
  EXPECT_EQ(trim("\t\nworld\r "), "world");
  EXPECT_EQ(trim("   "), "");
  EXPECT_EQ(trim(""), "");
  EXPECT_EQ(trim("no_trim"), "no_trim");
  EXPECT_EQ(trim("  leading"), "leading");
  EXPECT_EQ(trim("trailing  "), "trailing");
}

} // namespace zc::tests
