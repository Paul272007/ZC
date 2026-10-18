#pragma once

#include "framework/util/helpers.hpp"
#include "framework/util/types.hpp"

namespace zc::core
{

enum class RegistryTargetType : u8 {
  Bin,
  Lib,
  Header,
  Undef,
  // TODO: add TemplateCollection, ProjectTemplate, Test
};

inline std::string_view target_type_fmt(const RegistryTargetType type)
{
  switch (type) {
  case RegistryTargetType::Bin:
    return "Executable";
  case RegistryTargetType::Lib:
    return "Library";
  case RegistryTargetType::Header:
    return "Header-only library";
  case RegistryTargetType::Undef:
  default:
    return "Undefined";
  }
}

inline void from_json(const json &j, RegistryTargetType &p)
{
  p = from_str<RegistryTargetType>(j.get<std::string>()).value_or(RegistryTargetType::Undef);
}

inline void to_json(json &j, const RegistryTargetType &p)
{
  j = to_str(p).value_or("Undef");
}

} // namespace zc::core
