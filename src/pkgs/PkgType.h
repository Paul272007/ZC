/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "../helpers.h"

namespace zc
{

enum PkgType
{
  BIN,
  LIB,
  HEADER,
  COMPOSE,
  UNDEF
};

inline std::string pkg_type_to_pretty_str(const PkgType type)
{
  switch (type)
  {
  case BIN:
    return "Binary";
  case LIB:
    return "Library";
  case HEADER:
    return "Header-only library";
  case COMPOSE:
    return "Composed package";
  default:
    return "Undefined";
  }
}

inline std::string pkg_type_to_str(const PkgType type)
{
  switch (type)
  {
  case BIN:
    return "BIN";
  case LIB:
    return "LIB";
  case HEADER:
    return "HEADER";
  case COMPOSE:
    return "COMPOSE";
  default:
    return "UNDEF";
  }
}

inline PkgType pkg_type_from_str(const std::string &type_str)
{
  const std::string type_upper = upper(type_str);
  if (type_upper == "BIN")
    return BIN;
  if (type_upper == "LIB")
    return LIB;
  if (type_upper == "HEADER")
    return HEADER;
  if (type_upper == "COMPOSE")
    return COMPOSE;
  return UNDEF;
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
