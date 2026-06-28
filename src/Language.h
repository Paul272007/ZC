#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "helpers.h"

namespace zc
{

// clang-format off
#define ZC_SUPPORTED_LANGUAGES(X)                      \
  X(C,        "C",        "C")                         \
  X(CXX,      "CXX",      "CPP", "CC", "CXX", "C++")   \
  X(H,        "H",        "H")                         \
  X(HXX,      "HXX",      "HPP", "HH", "HXX", "H++")   \
  X(ASM_NASM, "ASM_NASM", "S", "ASM")                  \
  X(SH,       "SH",       "SH")

enum Language
{
#define GENERATE_ENUM(lang, name, ...) lang,
  ZC_SUPPORTED_LANGUAGES(GENERATE_ENUM)
#undef GENERATE_ENUM
  UNKNOWN_LANGUAGE,
};

// clang-format on

inline std::vector<std::string> extensions_for_language(const Language l)
{
  switch (l)
  {
#define RETURN_EXTS(lang, name, ...) \
  case lang:                         \
    return { __VA_ARGS__ };
    ZC_SUPPORTED_LANGUAGES(RETURN_EXTS)
#undef RETURN_EXTS
  default:
    return {};
  }
}

inline Language language_from_str(const std::string &txt)
{
  const auto upper_txt = upper(txt);

#define CHECK_EXTS(lang, name, ...)                  \
  for (const std::string_view ext : { __VA_ARGS__ }) \
    if (upper_txt == ext)                            \
      return lang;

  ZC_SUPPORTED_LANGUAGES(CHECK_EXTS)
#undef CHECK_EXTS

  return UNKNOWN_LANGUAGE;
}

inline bool is_of_language(Language l, const std::filesystem::path &file)
{
  std::string ext = file.extension().string();

  if (ext.empty())
    return false;

  ext.erase(0, 1);
  return language_from_str(ext) == l;
}

inline Language language_of(const std::filesystem::path &file)
{
  std::string ext = file.extension().string();

  if (ext.empty())
    return UNKNOWN_LANGUAGE;

  ext.erase(0, 1);
  return language_from_str(ext);
}

inline std::string language_to_str(const Language l)
{
  switch (l)
  {
#define RETURN_NAME(lang, name, ...) \
  case lang:                         \
    return name;
    ZC_SUPPORTED_LANGUAGES(RETURN_NAME)
#undef RETURN_NAME
  default:
    return "UNKNOWN_LANGUAGE";
  }
}

inline void from_json(const nlohmann::json &j, Language &l)
{
  l = language_from_str(j.get<std::string>());
}

inline void to_json(nlohmann::json &j, const Language &l)
{
  j = language_to_str(l);
}

} // namespace zc
