#pragma once

#include <string>
#include <string_view>

#include "framework/util/types.hpp"

namespace zc
{

enum class Style : u32 {
  None = 0,

  // Styles
  Bold       = 1U << 0,
  Faint      = 1U << 1,
  Italic     = 1U << 2,
  Underlined = 1U << 3,
  SBlink     = 1U << 4,
  FBlink     = 1U << 5,
  Invert     = 1U << 6,
  Conceal    = 1U << 7,
  CrossedOut = 1U << 8,

  // Colors
  White  = 1U << 9,
  Black  = 1U << 10,
  Red    = 1U << 11,
  Green  = 1U << 12,
  Yellow = 1U << 13,
  Blue   = 1U << 14,
  Purple = 1U << 15,
  Cyan   = 1U << 16,
};

constexpr Style operator|(Style a, Style b)
{
  return static_cast<Style>(std::to_underlying(a) | std::to_underlying(b));
}

constexpr bool has_style(Style mask, Style flag)
{
  return (std::to_underlying(mask) & std::to_underlying(flag)) != 0;
}

namespace detail
{

inline std::string ansi_seq(Style s)
{
  if (s == Style::None)
    return {};

  static constexpr std::pair<Style, std::string_view> table[] = {
    { Style::Bold, "1" },       { Style::Faint, "2" },  { Style::Italic, "3" },  { Style::Underlined, "4" },
    { Style::SBlink, "5" },     { Style::FBlink, "6" }, { Style::Invert, "7" },  { Style::Conceal, "8" },
    { Style::CrossedOut, "9" }, { Style::Black, "30" }, { Style::Red, "31" },    { Style::Green, "32" },
    { Style::Yellow, "33" },    { Style::Blue, "34" },  { Style::Purple, "35" }, { Style::Cyan, "36" },
    { Style::White, "37" },
  };

  std::string out   = "\033[";
  bool        first = true;
  for (const auto &[flag, code] : table) {
    if (has_style(s, flag)) {
      if (!first)
        out += ';';
      out   += code;
      first  = false;
    }
  }
  out += 'm';
  return out;
}

} // namespace detail

} // namespace zc
