#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>

#include "helpers.h"

// clang-format off
#define C_EXTENSIONS    {"C"}
#define H_EXTENSIONS    {"H"}
#define CXX_EXTENSIONS  {"CPP", "CC", "CXX", "C++"}
#define HXX_EXTENSIONS  {"HPP", "HH", "HXX", "H++"}
#define ASM_EXTENSIONS  {"S", "ASM"}
// clang-format on

namespace zc
{

enum Language
{
  C,
  CXX,
  H,
  HXX,
  ASM_NASM,
  UNKNOWN_LANGUAGE,
};

inline std::vector<std::string> extensions_for_language(const Language l)
{
  switch (l)
  {
  case C:
    return C_EXTENSIONS;
  case CXX:
    return CXX_EXTENSIONS;
  case H:
    return H_EXTENSIONS;
  case HXX:
    return HXX_EXTENSIONS;
  case ASM_NASM:
    return ASM_EXTENSIONS;
  default:
    return {};
  }
}

inline Language language_from_str(const std::string &txt)
{
  const auto upper_txt = upper(txt);
  for (const auto &ext : C_EXTENSIONS)
    if (upper_txt == ext)
      return C;

  for (const auto &ext : CXX_EXTENSIONS)
    if (upper_txt == ext)
      return CXX;

  for (const auto &ext : H_EXTENSIONS)
    if (upper_txt == ext)
      return H;

  for (const auto &ext : HXX_EXTENSIONS)
    if (upper_txt == ext)
      return HXX;

  for (const auto &ext : ASM_EXTENSIONS)
    if (upper_txt == ext)
      return ASM_NASM;

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
  case C:
    return "C";
  case CXX:
    return "CXX";
  case H:
    return "H";
  case HXX:
    return "HXX";
  case ASM_NASM:
    return "ASM_NASM";
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
