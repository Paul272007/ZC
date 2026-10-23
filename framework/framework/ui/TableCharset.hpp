#pragma once
#include <string_view>

#include "framework/util/types.hpp"

namespace zc
{

enum class Thickness : u8 {
  None,
  Single,
  Rounded,
  Bold,
  Double,
};

struct TableCharset {
  std::string_view horiz;
  std::string_view vert;
  std::string_view top_left, top_right, bottom_left, bottom_right;
  std::string_view tee_left, tee_right, tee_top, tee_bottom;
  std::string_view cross;
};

inline constexpr TableCharset chars[5][5] = {
  // H = None
  {
    // V = None
    {
      .horiz        = " ",
      .vert         = " ",
      .top_left     = " ",
      .top_right    = " ",
      .bottom_left  = " ",
      .bottom_right = " ",
      .tee_left     = " ",
      .tee_right    = " ",
      .tee_top      = " ",
      .tee_bottom   = " ",
      .cross        = " ",
    },
    // V = Single
    {
      .horiz        = " ",
      .vert         = "│",
      .top_left     = " ",
      .top_right    = " ",
      .bottom_left  = " ",
      .bottom_right = " ",
      .tee_left     = "│",
      .tee_right    = "│",
      .tee_top      = " ",
      .tee_bottom   = " ",
      .cross        = "│",
    },
    // V = Rounded
    {
      .horiz        = " ",
      .vert         = "│",
      .top_left     = " ",
      .top_right    = " ",
      .bottom_left  = " ",
      .bottom_right = " ",
      .tee_left     = "│",
      .tee_right    = "│",
      .tee_top      = " ",
      .tee_bottom   = " ",
      .cross        = "│",
    },
    // V = Bold
    {
      .horiz        = " ",
      .vert         = "┃",
      .top_left     = " ",
      .top_right    = " ",
      .bottom_left  = " ",
      .bottom_right = " ",
      .tee_left     = "┃",
      .tee_right    = "┃",
      .tee_top      = " ",
      .tee_bottom   = " ",
      .cross        = "┃",
    },
    // V = Double
    {
      .horiz        = " ",
      .vert         = "║",
      .top_left     = " ",
      .top_right    = " ",
      .bottom_left  = " ",
      .bottom_right = " ",
      .tee_left     = "║",
      .tee_right    = "║",
      .tee_top      = " ",
      .tee_bottom   = " ",
      .cross        = "║",
    },
  },
  // H = Single
  {
    // V = None
    {
      .horiz        = "─",
      .vert         = " ",
      .top_left     = "─",
      .top_right    = "─",
      .bottom_left  = "─",
      .bottom_right = "─",
      .tee_left     = "─",
      .tee_right    = "─",
      .tee_top      = "─",
      .tee_bottom   = "─",
      .cross        = "─",
    },
    // V = Single
    {
      .horiz        = "─",
      .vert         = "│",
      .top_left     = "┌",
      .top_right    = "┐",
      .bottom_left  = "└",
      .bottom_right = "┘",
      .tee_left     = "├",
      .tee_right    = "┤",
      .tee_top      = "┬",
      .tee_bottom   = "┴",
      .cross        = "┼",
    },
    // V = Rounded
    {
      .horiz        = "─",
      .vert         = "│",
      .top_left     = "╭",
      .top_right    = "╮",
      .bottom_left  = "╰",
      .bottom_right = "╯",
      .tee_left     = "├",
      .tee_right    = "┤",
      .tee_top      = "┬",
      .tee_bottom   = "┴",
      .cross        = "┼",
    },
    // V = Bold
    {
      .horiz        = "─",
      .vert         = "┃",
      .top_left     = "┎",
      .top_right    = "┒",
      .bottom_left  = "┖",
      .bottom_right = "┚",
      .tee_left     = "┠",
      .tee_right    = "┨",
      .tee_top      = "╁",
      .tee_bottom   = "╿",
      .cross        = "╂",
    },
    // V = Double
    {
      .horiz        = "─",
      .vert         = "║",
      .top_left     = "╓",
      .top_right    = "╖",
      .bottom_left  = "╙",
      .bottom_right = "╜",
      .tee_left     = "╟",
      .tee_right    = "╢",
      .tee_top      = "╥",
      .tee_bottom   = "╨",
      .cross        = "╫",
    },
  },
  // H = Rounded
  {
    // V = None
    {
      .horiz        = "─",
      .vert         = " ",
      .top_left     = "─",
      .top_right    = "─",
      .bottom_left  = "─",
      .bottom_right = "─",
      .tee_left     = "─",
      .tee_right    = "─",
      .tee_top      = "─",
      .tee_bottom   = "─",
      .cross        = "─",
    },
    // V = Single
    {
      .horiz        = "─",
      .vert         = "│",
      .top_left     = "╭",
      .top_right    = "╮",
      .bottom_left  = "╰",
      .bottom_right = "╯",
      .tee_left     = "├",
      .tee_right    = "┤",
      .tee_top      = "┬",
      .tee_bottom   = "┴",
      .cross        = "┼",
    },
    // V = Rounded
    {
      .horiz        = "─",
      .vert         = "│",
      .top_left     = "╭",
      .top_right    = "╮",
      .bottom_left  = "╰",
      .bottom_right = "╯",
      .tee_left     = "├",
      .tee_right    = "┤",
      .tee_top      = "┬",
      .tee_bottom   = "┴",
      .cross        = "┼",
    },
    // V = Bold
    {
      .horiz        = "─",
      .vert         = "┃",
      .top_left     = "┎",
      .top_right    = "┒",
      .bottom_left  = "┖",
      .bottom_right = "┚",
      .tee_left     = "┠",
      .tee_right    = "┨",
      .tee_top      = "╁",
      .tee_bottom   = "╿",
      .cross        = "╂",
    },
    // V = Double
    {
      .horiz        = "─",
      .vert         = "║",
      .top_left     = "╓",
      .top_right    = "╖",
      .bottom_left  = "╙",
      .bottom_right = "╜",
      .tee_left     = "╟",
      .tee_right    = "╢",
      .tee_top      = "╥",
      .tee_bottom   = "╨",
      .cross        = "╫",
    },
  },
  // H = Bold
  {
    // V = None
    {
      .horiz        = "━",
      .vert         = " ",
      .top_left     = "━",
      .top_right    = "━",
      .bottom_left  = "━",
      .bottom_right = "━",
      .tee_left     = "━",
      .tee_right    = "━",
      .tee_top      = "━",
      .tee_bottom   = "━",
      .cross        = "━",
    },
    // V = Single
    {
      .horiz        = "━",
      .vert         = "│",
      .top_left     = "┍",
      .top_right    = "┑",
      .bottom_left  = "┕",
      .bottom_right = "┙",
      .tee_left     = "┝",
      .tee_right    = "┥",
      .tee_top      = "┯",
      .tee_bottom   = "┷",
      .cross        = "┿",
    },
    // V = Rounded
    {
      .horiz        = "━",
      .vert         = "│",
      .top_left     = "┍",
      .top_right    = "┑",
      .bottom_left  = "┕",
      .bottom_right = "┙",
      .tee_left     = "┝",
      .tee_right    = "┥",
      .tee_top      = "┯",
      .tee_bottom   = "┷",
      .cross        = "┿",
    },
    // V = Bold
    {
      .horiz        = "━",
      .vert         = "┃",
      .top_left     = "┏",
      .top_right    = "┓",
      .bottom_left  = "┗",
      .bottom_right = "┛",
      .tee_left     = "┣",
      .tee_right    = "┫",
      .tee_top      = "┳",
      .tee_bottom   = "┻",
      .cross        = "╋",
    },
    // V = Double (fallback to Bold)
    {
      .horiz        = "━",
      .vert         = "┃",
      .top_left     = "┏",
      .top_right    = "┓",
      .bottom_left  = "┗",
      .bottom_right = "┛",
      .tee_left     = "┣",
      .tee_right    = "┫",
      .tee_top      = "┳",
      .tee_bottom   = "┻",
      .cross        = "╋",
    },
  },
  // H = Double
  {
    // V = None
    {
      .horiz        = "═",
      .vert         = " ",
      .top_left     = "═",
      .top_right    = "═",
      .bottom_left  = "═",
      .bottom_right = "═",
      .tee_left     = "═",
      .tee_right    = "═",
      .tee_top      = "═",
      .tee_bottom   = "═",
      .cross        = "═",
    },
    // V = Single
    {
      .horiz        = "═",
      .vert         = "│",
      .top_left     = "╒",
      .top_right    = "╕",
      .bottom_left  = "╘",
      .bottom_right = "╛",
      .tee_left     = "╞",
      .tee_right    = "╡",
      .tee_top      = "╤",
      .tee_bottom   = "╧",
      .cross        = "╪",
    },
    // V = Rounded
    {
      .horiz        = "═",
      .vert         = "│",
      .top_left     = "╒",
      .top_right    = "╕",
      .bottom_left  = "╘",
      .bottom_right = "╛",
      .tee_left     = "╞",
      .tee_right    = "╡",
      .tee_top      = "╤",
      .tee_bottom   = "╧",
      .cross        = "╪",
    },
    // V = Bold (fallback to Double)
    {
      .horiz        = "═",
      .vert         = "║",
      .top_left     = "╔",
      .top_right    = "╗",
      .bottom_left  = "╚",
      .bottom_right = "╝",
      .tee_left     = "╠",
      .tee_right    = "╣",
      .tee_top      = "╦",
      .tee_bottom   = "╩",
      .cross        = "╬",
    },
    // V = Double
    {
      .horiz        = "═",
      .vert         = "║",
      .top_left     = "╔",
      .top_right    = "╗",
      .bottom_left  = "╚",
      .bottom_right = "╝",
      .tee_left     = "╠",
      .tee_right    = "╣",
      .tee_top      = "╦",
      .tee_bottom   = "╩",
      .cross        = "╬",
    },
  },
};

constexpr const TableCharset &box(Thickness h, Thickness v) noexcept
{
  return chars[static_cast<u8>(h)][static_cast<u8>(v)];
}

} // namespace zc
