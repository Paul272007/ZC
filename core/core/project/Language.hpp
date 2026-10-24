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

using SourceMap = std::map<Language, std::vector<stdfs::path>>;

inline void to_json(json &p, const Language l)
{
  p = to_str<Language>(l).value_or("Unknown");
}

inline void from_json(const json &p, Language &l)
{
  l = from_str<Language>(p.get<std::string>()).value_or(Language::Unknown);
}

inline Language language_from_str(const std::string_view str)
{
  if (str == "c")
    return Language::C;
  if (equals_any(str, "cpp", "cc", "cxx", "c++"))
    return Language::CXX;
  return Language::Unknown;
}

inline Language language_of(const stdfs::path &file)
{
  auto ext = file.extension().string();
  if (ext.empty())
    return Language::Unknown;

  ext.erase(0, 1); // remove '.'
  return language_from_str(ext);
}

} // namespace zc::core

template<>
struct std::formatter<zc::core::Language> : std::formatter<std::string_view> {
  template<typename FormatContext>
  auto format(const zc::core::Language &l, FormatContext &ctx) const
  {
    return std::formatter<std::string_view>::format(zc::to_str(l).value_or("Unknown"), ctx);
  }
};
