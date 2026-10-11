#pragma once

#include <cstdio>

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
    // Deactivate canonic mode (=waiting for <Enter> key) and the echo.
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

enum class Key : u8 {
  None,
  Up,
  Down,
  Enter,
  Space,
};

inline Key read_key()
{
  const char c = get_char_raw();
  if (c == '\n' || c == '\r')
    return Key::Enter;
  if (c == ' ' || c == 'x')
    return Key::Space;

#if defined(_WIN32) || defined(_WIN64)
  if (c == -32 || c == 0 || static_cast<unsigned char>(c) == 224) {
    const char dir = get_char_raw();
    if (dir == 72)
      return Key::Up;
    if (dir == 80)
      return Key::Down;
  }
#else
  if (c == '\033') {
    if (const char bracket = get_char_raw(); bracket == '[') {
      const char dir = get_char_raw();
      if (dir == 'A')
        return Key::Up;
      if (dir == 'B')
        return Key::Down;
    }
  }
#endif
  return Key::None;
}

} // namespace zc
