#include <gtest/gtest.h>

#include "framework/util/helpers.hpp"

namespace zc::tests
{

enum class Color {
  Red,
  Green,
  Blue,
};

TEST(HelpersTest, EqualsAny)
{
  EXPECT_TRUE(equals_any(5, 1, 2, 5, 8));
  EXPECT_FALSE(equals_any(5, 1, 2, 3));
  EXPECT_TRUE(equals_any(std::string_view("apple"), "banana", "apple", "cherry"));
  EXPECT_FALSE(equals_any(std::string_view("orange"), "banana", "apple"));
}

TEST(HelpersTest, IsFlag)
{
  EXPECT_TRUE(is_flag("-h"));
  EXPECT_TRUE(is_flag("--help"));
  EXPECT_TRUE(is_flag("-v"));
  EXPECT_FALSE(is_flag("help"));
  EXPECT_FALSE(is_flag(""));
  EXPECT_FALSE(is_flag("file.txt"));
}

TEST(HelpersTest, ToBool)
{
  EXPECT_TRUE(to_bool("1"));
  EXPECT_TRUE(to_bool("true"));
  EXPECT_TRUE(to_bool("yes"));
  EXPECT_TRUE(to_bool("on"));
  EXPECT_FALSE(to_bool("0"));
  EXPECT_FALSE(to_bool("false"));
  EXPECT_FALSE(to_bool("no"));
  EXPECT_FALSE(to_bool("off"));
  EXPECT_FALSE(to_bool("anything_else"));
}

TEST(HelpersTest, ParseMode)
{
  EXPECT_EQ(
    parse_mode<Color>({ { Color::Red, false }, { Color::Green, false } }, Color::Blue, "Conflict"),
    Color::Blue
  );
  EXPECT_EQ(
    parse_mode<Color>({ { Color::Red, true }, { Color::Green, false } }, Color::Blue, "Conflict"),
    Color::Red
  );
  EXPECT_EQ(
    parse_mode<Color>({ { Color::Red, false }, { Color::Green, true } }, Color::Blue, "Conflict"),
    Color::Green
  );

  EXPECT_THROW(
    parse_mode<Color>({ { Color::Red, true }, { Color::Green, true } }, Color::Blue, "Conflicting flags"),
    Exception
  );
}

TEST(HelpersTest, Merge)
{
  std::vector<int>       dest          = { 1, 2, 3 };
  const std::vector<int> src           = { 3, 4, 5, 1 };
  const std::vector<int> expected_dest = { 1, 2, 3, 4, 5 };
  merge(src, dest);
  EXPECT_EQ(dest, expected_dest);
}

TEST(HelpersTest, CastVector)
{
  const std::vector<int>    src      = { 1, 2, 3 };
  auto                      dest     = cast_vector<int, double>(src);
  const std::vector<double> expected = { 1.0, 2.0, 3.0 };
  EXPECT_EQ(dest, expected);
}

TEST(HelpersTest, MakeStaticArray)
{
  constexpr auto arr = make_static_array<int, 3>([] { return std::array<int, 3>{ 10, 20, 30 }; });
  static_assert(arr[0] == 10);
  static_assert(arr[1] == 20);
  static_assert(arr[2] == 30);
  EXPECT_EQ(arr[0], 10);
}

TEST(HelpersTest, EnumStrConversion)
{
  // Under clang this returns dummy or default values as defined in helpers.hpp
  auto res = to_str(Color::Red);
  EXPECT_TRUE(res.has_value());
  auto from = from_str<Color>("Red");
  EXPECT_TRUE(from.has_value());
}

} // namespace zc::tests
