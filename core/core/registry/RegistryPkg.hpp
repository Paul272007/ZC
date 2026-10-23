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
    { "origin", p.origin },
    { "default", p.default_version },
  };
  if (!p.path.empty())
    j["path"] = p.path;

  j["versions"] = json::object();
  for (const auto &v : p.versions)
    j["versions"][v.version.string()] = v;
}

inline void from_json(const json &j, RegistryPkg &p)
{
  get_key(j, "origin", p.origin);
  get_key(j, "default", p.default_version);
  get_optional_key(j, "path", p.path);

  if (jsonContainsObject(j, "versions")) {
    for (const auto &[k, v] : j["versions"].items()) {
      auto snapshot    = v.get<RegistryProject>();
      snapshot.version = k;
      p.versions.push_back(std::move(snapshot));
    }
  }
}

} // namespace zc::core
