/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>

#include "../Version.h"
#include "../config/Conf.h"
#include "PkgType.h"

namespace zc
{

struct RegistryPkg
{
  std::string name;
  std::string target;
  std::string origin = "main";
  PkgType type = UNDEF;
  std::vector<Version> versions;

  bool operator==(const RegistryPkg &) const = default;
};

inline void to_json(nlohmann::json &j, const RegistryPkg &p)
{
  j = nlohmann::json{{"type", p.type}, {"target", p.target}, {"origin", p.origin}, {"versions", p.versions}};
}

inline void from_json(const nlohmann::json &j, RegistryPkg &p)
{
  Conf::get_key(j, "type", p.type);
  Conf::get_key(j, "target", p.target);
  Conf::get_key(j, "origin", p.origin);
  Conf::get_key(j, "versions", p.versions);
}

struct Pkg
{
  std::string name;
  std::string target;
  PkgType type = UNDEF;
  Version version;

  bool operator==(const Pkg &) const = default;
};

} // namespace zc
