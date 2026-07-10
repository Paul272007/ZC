#pragma once

#include <cstdint>
#include <string>

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"

namespace zc
{

enum class BuildMode : std::uint8_t
{
  automatic,
  release,
  debug,
};

inline std::string build_mode_to_str(BuildMode mode)
{
  switch (mode)
  {
  case BuildMode::release:
    return "release";
  case BuildMode::debug:
    return "debug";
  default:
    return "";
  }
}

inline BuildMode build_mode_from_str(const std::string &str)
{
  const auto upper_str = upper(str);
  if (upper_str == "RELEASE")
    return BuildMode::release;
  if (upper_str == "DEBUG")
    return BuildMode::debug;
  throw ZCException(ZCE_CONTENT_ERROR, "Invalid build mode declaration.");
}

} // namespace zc
