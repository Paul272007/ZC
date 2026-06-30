#pragma once

#include <string>

#include "../helpers.h"
#include "../Version.h"
#include "PkgType.h"

namespace zc
{

struct Pkg
{
  std::string name;
  std::string target;
  std::string origin = "main";

  PkgType type = PkgType::UNDEF;
  Version default_version;

  std::vector<Version> versions;

  bool operator==(const Pkg &) const = default;
};

inline void to_json(nlohmann::json &j, const Pkg &p)
{
  j = nlohmann::json{
    { "type", p.type },         { "target", p.target },
    { "origin", p.origin },     { "default", p.default_version },
    { "versions", p.versions },
  };
}

inline void from_json(const nlohmann::json &j, Pkg &p)
{
  get_key(j, "type", p.type);
  get_key(j, "target", p.target);
  get_key(j, "origin", p.origin);
  get_key(j, "default", p.default_version);
  get_key(j, "versions", p.versions);
}

} // namespace zc
