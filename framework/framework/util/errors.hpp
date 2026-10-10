#pragma once

#include "types.hpp"

namespace zc
{

enum class FSError : u8 {
  FileNotFound = 10,
  DirNotFound  = 11,
  NotAFile     = 12,
  NotADir      = 13,
  ReadingError = 14,
  WritingError = 15,
  ParsingError = 16,
};

enum class FormatError : u8 {
  InvalidVersion = 20,
  InvalidEnum    = 21,
  InvalidStr     = 22,
};

} // namespace zc
