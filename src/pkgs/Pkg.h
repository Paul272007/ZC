#pragma once

#include <string>

#include "../helpers.h"
#include "../Version.h"
#include "PkgType.h"

namespace zc
{

struct Pkg
{
  std::string path;
  std::string name;
  std::string target;
  std::string origin = "main";

  PkgType type = PkgType::UNDEF;
  Version default_version;

  std::map<Version, std::map<std::string, Version>> versions;

  bool operator==(const Pkg &) const = default;
};

inline void to_json(nlohmann::json &j, const Pkg &p)
{
  std::map<std::string, std::map<std::string, Version>> string_versions;
  for (const auto& [v, deps] : p.versions) {
    string_versions[v.string()] = deps;
  }

  j = nlohmann::json{
    { "type", p.type },         { "target", p.target },
    { "origin", p.origin },     { "default", p.default_version },
    { "versions", string_versions },
  };
  if (!p.path.empty())
    j["path"] = p.path;
}

inline void from_json(const nlohmann::json &j, Pkg &p)
{
  get_key(j, "type", p.type);
  get_key(j, "target", p.target);
  get_key(j, "origin", p.origin);
  get_key(j, "default", p.default_version);
  get_key(j, "path", p.path, p.path);
  std::map<std::string, std::map<std::string, Version>> string_versions;
  get_key(j, "versions", string_versions);
  for (const auto& [v_str, deps] : string_versions) {
    p.versions[Version(v_str)] = deps;
  }
}

} // namespace zc
