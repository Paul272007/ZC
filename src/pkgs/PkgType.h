#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "../helpers.h"

namespace zc
{

enum class PkgType
{
  BIN,
  LIB,
  HEADER,
  COMPOSE,
  UNDEF,
};

inline std::string pkg_type_to_pretty_str(const PkgType type)
{
  switch (type)
  {
  case PkgType::BIN:
    return "Binary";
  case PkgType::LIB:
    return "Library";
  case PkgType::HEADER:
    return "Header-only library";
  case PkgType::COMPOSE:
    return "Composed package";
  default:
    return "Undefined";
  }
}

inline std::string pkg_type_to_str(const PkgType type)
{
  switch (type)
  {
  case PkgType::BIN:
    return "BIN";
  case PkgType::LIB:
    return "LIB";
  case PkgType::HEADER:
    return "HEADER";
  case PkgType::COMPOSE:
    return "COMPOSE";
  default:
    return "UNDEF";
  }
}

inline PkgType pkg_type_from_str(const std::string &type_str)
{
  const std::string type_upper = upper(type_str);
  if (type_upper == "BIN")
    return PkgType::BIN;
  if (type_upper == "LIB")
    return PkgType::LIB;
  if (type_upper == "HEADER")
    return PkgType::HEADER;
  if (type_upper == "COMPOSE")
    return PkgType::COMPOSE;
  return PkgType::UNDEF;
}

inline void from_json(const nlohmann::json &j, PkgType &p)
{
  p = pkg_type_from_str(j.get<std::string>());
}

inline void to_json(nlohmann::json &j, const PkgType &p)
{
  j = pkg_type_to_str(p);
}

} // namespace zc
