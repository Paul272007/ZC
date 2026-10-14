#pragma once

#include "types.hpp"

namespace zc
{

enum class CliError : u8 {
  MissingSubCommand,
  MissingFlag,
  UnknownSubCommand, // unused
  UnknownFlag,
  MissingValue,      // unused
  MissingPositional,
  UnexpectedPositional,
  IncompatibleFlags,
  Interrupted,
  InvalidValue, // unused
};

constexpr u8 exit_code_base(CliError /**/) noexcept
{
  return 10;
}

enum class FSError : u8 {
  FileNotFound,
  DirNotFound,
  NotAFile,
  NotADir,
  ReadingError,
  WritingError,
  ParsingError,
};

constexpr u8 exit_code_base(FSError /**/) noexcept
{
  return 20;
}

enum class FormatError : u8 {
  InvalidVersion,
  InvalidEnum,
  InvalidStr,
};

constexpr u8 exit_code_base(FormatError /**/) noexcept
{
  return 30;
}

enum class OSError : u8 {
  PipeError,
};

constexpr u8 exit_code_base(OSError /**/) noexcept
{
  return 40;
}

} // namespace zc
