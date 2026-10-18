#pragma once

#include "framework/util/helpers.hpp"
#include "framework/util/types.hpp"

namespace zc::core
{

enum class Language : u8 {
  C,
  CXX,
  ASM,
  Unknown,
};

inline void to_json(json &p, const Language l)
{
  p = to_str<Language>(l).value_or("Unknown");
}

inline void from_json(const json &p, Language &l)
{
  l = from_str<Language>(p).value_or(Language::Unknown);
}

} // namespace zc::core
