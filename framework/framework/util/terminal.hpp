#pragma once

#include <format>
#include <string>
#include <string_view>

#include "types.hpp"

namespace zc
{

constexpr std::string cursor_up(u64 n)
{
  return std::format("\033[{}A", n);
}

constexpr std::string cursor_down(u64 n)
{
  return std::format("\033[{}B", n);
}

inline constexpr std::string_view cursor_top_left    = "\033[1;1H";
inline constexpr std::string_view clear_under_cursor = "\033[J";
inline constexpr std::string_view clear_screen       = "\033[2J";
inline constexpr std::string_view clear_line         = "\033[K";
inline constexpr std::string_view hide_cursor        = "\033[?25l";
inline constexpr std::string_view show_cursor        = "\033[?25h";

inline constexpr std::string_view reset       = "\033[0m";
inline constexpr std::string_view bold        = "\033[1m";
inline constexpr std::string_view faint       = "\033[2m";
inline constexpr std::string_view italic      = "\033[3m";
inline constexpr std::string_view ul          = "\033[4m";
inline constexpr std::string_view s_blink     = "\033[5m";
inline constexpr std::string_view f_blink     = "\033[6m";
inline constexpr std::string_view invert      = "\033[7m";
inline constexpr std::string_view conceal     = "\033[8m";
inline constexpr std::string_view crossed_out = "\033[9m";

inline constexpr std::string_view black  = "\033[0;30m";
inline constexpr std::string_view red    = "\033[0;31m";
inline constexpr std::string_view green  = "\033[0;32m";
inline constexpr std::string_view yellow = "\033[0;33m";
inline constexpr std::string_view blue   = "\033[0;34m";
inline constexpr std::string_view purple = "\033[0;35m";
inline constexpr std::string_view cyan   = "\033[0;36m";
inline constexpr std::string_view white  = "\033[0;37m";

inline constexpr std::string_view b_black  = "\033[1;30m";
inline constexpr std::string_view b_red    = "\033[1;31m";
inline constexpr std::string_view b_green  = "\033[1;32m";
inline constexpr std::string_view b_yellow = "\033[1;33m";
inline constexpr std::string_view b_blue   = "\033[1;34m";
inline constexpr std::string_view b_purple = "\033[1;35m";
inline constexpr std::string_view b_cyan   = "\033[1;36m";
inline constexpr std::string_view b_white  = "\033[1;37m";

inline constexpr std::string_view u_black  = "\033[4;30m";
inline constexpr std::string_view u_red    = "\033[4;31m";
inline constexpr std::string_view u_green  = "\033[4;32m";
inline constexpr std::string_view u_yellow = "\033[4;33m";
inline constexpr std::string_view u_blue   = "\033[4;34m";
inline constexpr std::string_view u_purple = "\033[4;35m";
inline constexpr std::string_view u_cyan   = "\033[4;36m";
inline constexpr std::string_view u_white  = "\033[4;37m";

inline constexpr std::string_view bg_black  = "\033[40m";
inline constexpr std::string_view bg_red    = "\033[41m";
inline constexpr std::string_view bg_green  = "\033[42m";
inline constexpr std::string_view bg_yellow = "\033[43m";
inline constexpr std::string_view bg_blue   = "\033[44m";
inline constexpr std::string_view bg_purple = "\033[45m";
inline constexpr std::string_view bg_cyan   = "\033[46m";
inline constexpr std::string_view bg_white  = "\033[47m";

inline constexpr std::string_view h_black  = "\033[0;90m";
inline constexpr std::string_view h_red    = "\033[0;91m";
inline constexpr std::string_view h_green  = "\033[0;92m";
inline constexpr std::string_view h_yellow = "\033[0;93m";
inline constexpr std::string_view h_blue   = "\033[0;94m";
inline constexpr std::string_view h_purple = "\033[0;95m";
inline constexpr std::string_view h_cyan   = "\033[0;96m";
inline constexpr std::string_view h_white  = "\033[0;97m";

inline constexpr std::string_view hb_black  = "\033[1;90m";
inline constexpr std::string_view hb_red    = "\033[1;91m";
inline constexpr std::string_view hb_green  = "\033[1;92m";
inline constexpr std::string_view hb_yellow = "\033[1;93m";
inline constexpr std::string_view hb_blue   = "\033[1;94m";
inline constexpr std::string_view hb_purple = "\033[1;95m";
inline constexpr std::string_view hb_cyan   = "\033[1;96m";
inline constexpr std::string_view hb_white  = "\033[1;97m";

inline constexpr std::string_view hbg_black  = "\033[0;100m";
inline constexpr std::string_view hbg_red    = "\033[0;101m";
inline constexpr std::string_view hbg_green  = "\033[0;102m";
inline constexpr std::string_view hbg_yellow = "\033[0;103m";
inline constexpr std::string_view hbg_blue   = "\033[0;104m";
inline constexpr std::string_view hbg_purple = "\033[0;105m";
inline constexpr std::string_view hbg_cyan   = "\033[0;106m";
inline constexpr std::string_view hbg_white  = "\033[0;107m";

// inline constexpr std::string_view light_vertical_left               = "\u2524";
// inline constexpr std::string_view vertical_single_left_double       = "\u2561";
// inline constexpr std::string_view vertical_double_left_single       = "\u2562";
// inline constexpr std::string_view down_double_left_single           = "\u2556";
// inline constexpr std::string_view down_single_left_double           = "\u2555";
// inline constexpr std::string_view double_vertical_left              = "\u2563";
// inline constexpr std::string_view double_vertical                   = "\u2551";
// inline constexpr std::string_view double_down_left                  = "\u2557";
// inline constexpr std::string_view double_up_left                    = "\u255D";
// inline constexpr std::string_view up_double_left_single             = "\u255C";
// inline constexpr std::string_view up_single_left_double             = "\u255B";
// inline constexpr std::string_view light_down_left                   = "\u2510";
// inline constexpr std::string_view light_up_right                    = "\u2514";
// inline constexpr std::string_view light_up_horizontal               = "\u2534";
// inline constexpr std::string_view light_down_horizontal             = "\u252C";
// inline constexpr std::string_view light_vertical_right              = "\u251C";
// inline constexpr std::string_view light_horizontal                  = "\u2500";
// inline constexpr std::string_view light_vertical                    = "\u2502";
// inline constexpr std::string_view light_vertical_horizontal         = "\u253C";
// inline constexpr std::string_view vertical_single_right_double      = "\u255E";
// inline constexpr std::string_view vertical_double_right_single      = "\u255F";
// inline constexpr std::string_view double_up_right                   = "\u255A";
// inline constexpr std::string_view double_down_right                 = "\u2554";
// inline constexpr std::string_view double_up_horizontal              = "\u2569";
// inline constexpr std::string_view double_down_horizontal            = "\u2566";
// inline constexpr std::string_view double_vertical_right             = "\u2560";
// inline constexpr std::string_view double_horizontal                 = "\u2550";
// inline constexpr std::string_view double_vertical_horizontal        = "\u256C";
// inline constexpr std::string_view up_single_horizontal_double       = "\u2567";
// inline constexpr std::string_view up_double_horizontal_single       = "\u2568";
// inline constexpr std::string_view down_single_horizontal_double     = "\u2564";
// inline constexpr std::string_view down_double_horizontal_single     = "\u2565";
// inline constexpr std::string_view up_double_right_single            = "\u2559";
// inline constexpr std::string_view up_single_right_double            = "\u2558";
// inline constexpr std::string_view down_single_right_double          = "\u2552";
// inline constexpr std::string_view down_double_right_single          = "\u2553";
// inline constexpr std::string_view vertical_double_horizontal_single = "\u256B";
// inline constexpr std::string_view vertical_single_horizontal_double = "\u256A";
// inline constexpr std::string_view light_up_left                     = "\u2518";
// inline constexpr std::string_view light_down_right                  = "\u250C";

} // namespace zc
