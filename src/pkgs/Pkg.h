#pragma once

#include <string>

#include "../helpers.h"
#include "../Version.h"
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
  j = nlohmann::json{
    {     "name",     p.name },
    {     "type",     p.type },
    {   "target",   p.target },
    {   "origin",   p.origin },
    { "versions", p.versions }
  };
}

inline void from_json(const nlohmann::json &j, RegistryPkg &p)
{
  get_key(j, "name", p.name);
  get_key(j, "type", p.type);
  get_key(j, "target", p.target);
  get_key(j, "origin", p.origin);
  get_key(j, "versions", p.versions);
}

} // namespace zc
