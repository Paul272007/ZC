#pragma once

#include <cstdio>
#include <string>

#include "types.hpp"

#ifdef ZC_WINDOWS
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif

namespace zc
{

inline void set_raw_mode(const bool enable)
{
#ifndef ZC_WINDOWS
  static termios oldt;
  static termios newt;
  if (enable) {
    tcgetattr(STDIN_FILENO, &oldt); // Save the current state of the terminal
    newt = oldt;
    // Deactivate canonic mode (=waiting for <Enter> key) and the echo. Leave ISIG enabled for SIGINT.
    newt.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new parameters
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore initial state
  }
#endif
}

inline char get_char_raw()
{
#ifdef ZC_WINDOWS
  return _getch(); // Read a character without waiting for the <Enter> key on Windows
#else
  return static_cast<char>(getchar());
#endif
}

inline std::string cursor_up(u64 n)
{
  return std::format("\033[{}A", n);
}

inline std::string cursor_down(u64 n)
{
  return std::format("\033[{}B", n);
}

constexpr std::string_view cursor_top_left    = "\033[1;1H";
constexpr std::string_view clear_under_cursor = "\033[J";
constexpr std::string_view clear_screen       = "\033[2J";
constexpr std::string_view clear_line         = "\033[K";
constexpr std::string_view hide_cursor        = "\033[?25l";
constexpr std::string_view show_cursor        = "\033[?25h";

constexpr std::string_view reset       = "\033[0m";
constexpr std::string_view bold        = "\033[1m";
constexpr std::string_view faint       = "\033[2m";
constexpr std::string_view italic      = "\033[3m";
constexpr std::string_view ul          = "\033[4m";
constexpr std::string_view s_blink     = "\033[5m";
constexpr std::string_view f_blink     = "\033[6m";
constexpr std::string_view invert      = "\033[7m";
constexpr std::string_view conceal     = "\033[8m";
constexpr std::string_view crossed_out = "\033[9m";

constexpr std::string_view black  = "\033[0;30m";
constexpr std::string_view red    = "\033[0;31m";
constexpr std::string_view green  = "\033[0;32m";
constexpr std::string_view yellow = "\033[0;33m";
constexpr std::string_view blue   = "\033[0;34m";
constexpr std::string_view purple = "\033[0;35m";
constexpr std::string_view cyan   = "\033[0;36m";
constexpr std::string_view white  = "\033[0;37m";

constexpr std::string_view b_black  = "\033[1;30m";
constexpr std::string_view b_red    = "\033[1;31m";
constexpr std::string_view b_green  = "\033[1;32m";
constexpr std::string_view b_yellow = "\033[1;33m";
constexpr std::string_view b_blue   = "\033[1;34m";
constexpr std::string_view b_purple = "\033[1;35m";
constexpr std::string_view b_cyan   = "\033[1;36m";
constexpr std::string_view b_white  = "\033[1;37m";

constexpr std::string_view u_black  = "\033[4;30m";
constexpr std::string_view u_red    = "\033[4;31m";
constexpr std::string_view u_green  = "\033[4;32m";
constexpr std::string_view u_yellow = "\033[4;33m";
constexpr std::string_view u_blue   = "\033[4;34m";
constexpr std::string_view u_purple = "\033[4;35m";
constexpr std::string_view u_cyan   = "\033[4;36m";
constexpr std::string_view u_white  = "\033[4;37m";

constexpr std::string_view bg_black  = "\033[40m";
constexpr std::string_view bg_red    = "\033[41m";
constexpr std::string_view bg_green  = "\033[42m";
constexpr std::string_view bg_yellow = "\033[43m";
constexpr std::string_view bg_blue   = "\033[44m";
constexpr std::string_view bg_purple = "\033[45m";
constexpr std::string_view bg_cyan   = "\033[46m";
constexpr std::string_view bg_white  = "\033[47m";

constexpr std::string_view h_black  = "\033[0;90m";
constexpr std::string_view h_red    = "\033[0;91m";
constexpr std::string_view h_green  = "\033[0;92m";
constexpr std::string_view h_yellow = "\033[0;93m";
constexpr std::string_view h_blue   = "\033[0;94m";
constexpr std::string_view h_purple = "\033[0;95m";
constexpr std::string_view h_cyan   = "\033[0;96m";
constexpr std::string_view h_white  = "\033[0;97m";

constexpr std::string_view hb_black  = "\033[1;90m";
constexpr std::string_view hb_red    = "\033[1;91m";
constexpr std::string_view hb_green  = "\033[1;92m";
constexpr std::string_view hb_yellow = "\033[1;93m";
constexpr std::string_view hb_blue   = "\033[1;94m";
constexpr std::string_view hb_purple = "\033[1;95m";
constexpr std::string_view hb_cyan   = "\033[1;96m";
constexpr std::string_view hb_white  = "\033[1;97m";

constexpr std::string_view hbg_black  = "\033[0;100m";
constexpr std::string_view hbg_red    = "\033[0;101m";
constexpr std::string_view hbg_green  = "\033[0;102m";
constexpr std::string_view hbg_yellow = "\033[0;103m";
constexpr std::string_view hbg_blue   = "\033[0;104m";
constexpr std::string_view hbg_purple = "\033[0;105m";
constexpr std::string_view hbg_cyan   = "\033[0;106m";
constexpr std::string_view hbg_white  = "\033[0;107m";

constexpr std::string_view light_vertical_left               = "\u2524";
constexpr std::string_view vertical_single_left_double       = "\u2561";
constexpr std::string_view vertical_double_left_single       = "\u2562";
constexpr std::string_view down_double_left_single           = "\u2556";
constexpr std::string_view down_single_left_double           = "\u2555";
constexpr std::string_view double_vertical_left              = "\u2563";
constexpr std::string_view double_vertical                   = "\u2551";
constexpr std::string_view double_down_left                  = "\u2557";
constexpr std::string_view double_up_left                    = "\u255D";
constexpr std::string_view up_double_left_single             = "\u255C";
constexpr std::string_view up_single_left_double             = "\u255B";
constexpr std::string_view light_down_left                   = "\u2510";
constexpr std::string_view light_up_right                    = "\u2514";
constexpr std::string_view light_up_horizontal               = "\u2534";
constexpr std::string_view light_down_horizontal             = "\u252C";
constexpr std::string_view light_vertical_right              = "\u251C";
constexpr std::string_view light_horizontal                  = "\u2500";
constexpr std::string_view light_vertical                    = "\u2502";
constexpr std::string_view light_vertical_horizontal         = "\u253C";
constexpr std::string_view vertical_single_right_double      = "\u255E";
constexpr std::string_view vertical_double_right_single      = "\u255F";
constexpr std::string_view double_up_right                   = "\u255A";
constexpr std::string_view double_down_right                 = "\u2554";
constexpr std::string_view double_up_horizontal              = "\u2569";
constexpr std::string_view double_down_horizontal            = "\u2566";
constexpr std::string_view double_vertical_right             = "\u2560";
constexpr std::string_view double_horizontal                 = "\u2550";
constexpr std::string_view double_vertical_horizontal        = "\u256C";
constexpr std::string_view up_single_horizontal_double       = "\u2567";
constexpr std::string_view up_double_horizontal_single       = "\u2568";
constexpr std::string_view down_single_horizontal_double     = "\u2564";
constexpr std::string_view down_double_horizontal_single     = "\u2565";
constexpr std::string_view up_double_right_single            = "\u2559";
constexpr std::string_view up_single_right_double            = "\u2558";
constexpr std::string_view down_single_right_double          = "\u2552";
constexpr std::string_view down_double_right_single          = "\u2553";
constexpr std::string_view vertical_double_horizontal_single = "\u256B";
constexpr std::string_view vertical_single_horizontal_double = "\u256A";
constexpr std::string_view light_up_left                     = "\u2518";
constexpr std::string_view light_down_right                  = "\u250C";

} // namespace zc
