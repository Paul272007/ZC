#pragma once

#include "core/registry/RegistryProject.hpp"
#include "core/util/json_helpers.hpp"
#include "framework/util/types.hpp"
#include "framework/Version.hpp"

namespace zc::core
{

using DepsMap = std::map<std::string, Version, std::less<>>;

struct RegistryPkg {
  stdfs::path path;
  std::string name;
  std::string origin{ "main" };
  Version     default_version;

  std::vector<RegistryProject> versions;

  bool operator==(const RegistryPkg &) const = default;
};

inline void to_json(json &j, const RegistryPkg &p)
{
  j = json{
    //{ "name", p.name },
    { "origin", p.origin },
    { "default", p.default_version },
    { "versions", p.versions },
  };
  if (!p.path.empty())
    j["path"] = p.path;
}

inline void from_json(const json &j, RegistryPkg &p)
{
  // get_key(j, "name", p.name); // initialized in Registry::load()
  get_key(j, "origin", p.origin);
  get_key(j, "default", p.default_version);
  get_key(j, "versions", p.versions);
  get_optional_key(j, "path", p.path);
}

} // namespace zc::core
