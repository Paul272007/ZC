#pragma once

#include <csignal>

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
  static bool    is_raw = false;

  if (enable && !is_raw) {
    if (tcgetattr(STDIN_FILENO, &oldt) == 0) {
      termios newt  = oldt;
      newt.c_lflag &= ~static_cast<tcflag_t>(ICANON | ECHO);
      if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) == 0)
        is_raw = true;
    }
  } else if (!enable && is_raw) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    is_raw = false;
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
  Esc,
};

inline Key read_key()
{
  const char c = get_char_raw();
  if (c == 3) { // Ctrl+C
    (void)std::raise(SIGINT);
    return Key::None;
  }
  if (c == '\n' || c == '\r')
    return Key::Enter;
  if (c == ' ' || c == 'x')
    return Key::Space;
  if (c == 'k')
    return Key::Up;
  if (c == 'j')
    return Key::Down;
  if (c == 'q') // TODO: or esc
    return Key::Esc;

#ifdef ZC_WINDOWS
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
