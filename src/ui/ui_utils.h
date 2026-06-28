#pragma once

#include <cstdio>
#if defined(_WIN32) || defined(_WIN64)
  #include <conio.h> // _getch()
#else
  #include <termios.h>
  #include <unistd.h>
#endif

namespace zc
{

inline void set_raw_mode(const bool enable)
{
#if !defined(_WIN32) && !defined(_WIN64)
  static termios oldt, newt;
  if (enable)
  {
    tcgetattr(STDIN_FILENO, &oldt); // Save the current state of the terminal
    newt = oldt;
    // Deactivate canonic mode (=waiting for <Enter> key) and the echo, deactivate ctrl+c = SIGINT
    newt.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Apply new parameters
  }
  else
  {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restore initial state
  }
#endif
}

inline char get_char_raw()
{
#if defined(_WIN32) || defined(_WIN64)
  return _getch(); // Read a character without waiting for the <Enter> key on Windows
#else
  return static_cast<char>(getchar());
#endif
}

#define CURSOR_UP(n)                      ("\033[" + std::to_string(n) + "A")
#define CURSOR_DOWN(n)                    ("\033[" + std::to_string(n) + "B")
#define CURSOR_TOP_LEFT                   "\033[1;1H"
#define CLEAR_UNDER_CURSOR                "\033[J"
#define CLEAR_SCREEN                      "\033[2J"
#define CLEAR_LINE                        "\033[K"
#define HIDE_CURSOR                       "\033[?25l"
#define SHOW_CURSOR                       "\033[?25h"

#define RESET                             "\033[0m"
#define BOLD                              "\033[1m"
#define FAINT                             "\033[2m"
#define ITALIC                            "\033[3m"
#define UL                                "\033[4m"
#define S_BLINK                           "\033[5m"
#define F_BLINK                           "\033[6m"
#define INVERT                            "\033[7m"
#define CONCEAL                           "\033[8m"
#define CROSSED_OUT                       "\033[9m"

#define BLACK                             "\033[0;30m"
#define RED                               "\033[0;31m"
#define GREEN                             "\033[0;32m"
#define YELLOW                            "\033[0;33m"
#define BLUE                              "\033[0;34m"
#define PURPLE                            "\033[0;35m"
#define CYAN                              "\033[0;36m"
#define WHITE                             "\033[0;37m"

#define B_BLACK                           "\033[1;30m"
#define B_RED                             "\033[1;31m"
#define B_GREEN                           "\033[1;32m"
#define B_YELLOW                          "\033[1;33m"
#define B_BLUE                            "\033[1;34m"
#define B_PURPLE                          "\033[1;35m"
#define B_CYAN                            "\033[1;36m"
#define B_WHITE                           "\033[1;37m"

#define U_BLACK                           "\033[4;30m"
#define U_RED                             "\033[4;31m"
#define U_GREEN                           "\033[4;32m"
#define U_YELLOW                          "\033[4;33m"
#define U_BLUE                            "\033[4;34m"
#define U_PURPLE                          "\033[4;35m"
#define U_CYAN                            "\033[4;36m"
#define U_WHITE                           "\033[4;37m"

#define BG_BLACK                          "\033[40m"
#define BG_RED                            "\033[41m"
#define BG_GREEN                          "\033[42m"
#define BG_YELLOW                         "\033[43m"
#define BG_BLUE                           "\033[44m"
#define BG_PURPLE                         "\033[45m"
#define BG_CYAN                           "\033[46m"
#define BG_WHITE                          "\033[47m"

#define H_BLACK                           "\033[0;90m"
#define H_RED                             "\033[0;91m"
#define H_GREEN                           "\033[0;92m"
#define H_YELLOW                          "\033[0;93m"
#define H_BLUE                            "\033[0;94m"
#define H_PURPLE                          "\033[0;95m"
#define H_CYAN                            "\033[0;96m"
#define H_WHITE                           "\033[0;97m"

#define HB_BLACK                          "\033[1;90m"
#define HB_RED                            "\033[1;91m"
#define HB_GREEN                          "\033[1;92m"
#define HB_YELLOW                         "\033[1;93m"
#define HB_BLUE                           "\033[1;94m"
#define HB_PURPLE                         "\033[1;95m"
#define HB_CYAN                           "\033[1;96m"
#define HB_WHITE                          "\033[1;97m"

#define HBG_BLACK                         "\033[0;100m"
#define HBG_RED                           "\033[0;101m"
#define HBG_GREEN                         "\033[0;102m"
#define HBG_YELLOW                        "\033[0;103m"
#define HBG_BLUE                          "\033[0;104m"
#define HBG_PURPLE                        "\033[0;105m"
#define HBG_CYAN                          "\033[0;106m"
#define HBG_WHITE                         "\033[0;107m"

#define LIGHT_VERTICAL_LEFT               "\u2524"
#define VERTICAL_SINGLE_LEFT_DOUBLE       "\u2561"
#define VERTICAL_DOUBLE_LEFT_SINGLE       "\u2562"
#define DOWN_DOUBLE_LEFT_SINGLE           "\u2556"
#define DOWN_SINGLE_LEFT_DOUBLE           "\u2555"
#define DOUBLE_VERTICAL_LEFT              "\u2563"
#define DOUBLE_VERTICAL                   "\u2551"
#define DOUBLE_DOWN_LEFT                  "\u2557"
#define DOUBLE_UP_LEFT                    "\u255D"
#define UP_DOUBLE_LEFT_SINGLE             "\u255C"
#define UP_SINGLE_LEFT_DOUBLE             "\u255B"
#define LIGHT_DOWN_LEFT                   "\u2510"
#define LIGHT_UP_RIGHT                    "\u2514"
#define LIGHT_UP_HORIZONTAL               "\u2534"
#define LIGHT_DOWN_HORIZONTAL             "\u252C"
#define LIGHT_VERTICAL_RIGHT              "\u251C"
#define LIGHT_HORIZONTAL                  "\u2500"
#define LIGHT_VERTICAL                    "\u2502"
#define LIGHT_VERTICAL_HORIZONTAL         "\u253C"
#define VERTICAL_SINGLE_RIGHT_DOUBLE      "\u255E"
#define VERTICAL_DOUBLE_RIGHT_SINGLE      "\u255F"
#define DOUBLE_UP_RIGHT                   "\u255A"
#define DOUBLE_DOWN_RIGHT                 "\u2554"
#define DOUBLE_UP_HORIZONTAL              "\u2569"
#define DOUBLE_DOWN_HORIZONTAL            "\u2566"
#define DOUBLE_VERTICAL_RIGHT             "\u2560"
#define DOUBLE_HORIZONTAL                 "\u2550"
#define DOUBLE_VERTICAL_HORIZONTAL        "\u256C"
#define UP_SINGLE_HORIZONTAL_DOUBLE       "\u2567"
#define UP_DOUBLE_HORIZONTAL_SINGLE       "\u2568"
#define DOWN_SINGLE_HORIZONTAL_DOUBLE     "\u2564"
#define DOWN_DOUBLE_HORIZONTAL_SINGLE     "\u2565"
#define UP_DOUBLE_RIGHT_SINGLE            "\u2559"
#define UP_SINGLE_RIGHT_DOUBLE            "\u2558"
#define DOWN_SINGLE_RIGHT_DOUBLE          "\u2552"
#define DOWN_DOUBLE_RIGHT_SINGLE          "\u2553"
#define VERTICAL_DOUBLE_HORIZONTAL_SINGLE "\u256B"
#define VERTICAL_SINGLE_HORIZONTAL_DOUBLE "\u256A"
#define LIGHT_UP_LEFT                     "\u2518"
#define LIGHT_DOWN_RIGHT                  "\u250C"

} // namespace zc
