#pragma once

#include <string>

#include "core/util/json_helpers.hpp"
#include "framework/Version.hpp"

namespace zc::core
{

struct RegistryTargetDependency {
  std::string package;
  Version     version;

  std::vector<std::string> targets;

  bool operator==(const RegistryTargetDependency &rhs) const = default;
};

inline void to_json(json &j, const RegistryTargetDependency &d)
{
  j = json{
    { "package", d.package },
    { "version", d.version },
    { "target", d.targets },
  };
}

inline void from_json(const json &j, RegistryTargetDependency &d)
{
  get_key(j, "package", d.package);
  get_key(j, "version", d.version);
  get_key(j, "target", d.targets);
}

} // namespace zc::core
