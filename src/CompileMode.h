#pragma once

#include <cstdint>

namespace zc
{

enum class CompileMode : std::uint8_t
{
  preprocess,
  compile,
  assemble,
  full,
};

} // namespace zc
