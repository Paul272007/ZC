#pragma once

#include <string>

#include "core/project/Link.hpp"
#include "core/registry/target/RegistryTargetDependency.hpp"
#include "core/util/json_helpers.hpp"
#include "framework/Version.hpp"

namespace zc::core
{

struct ProjectTargetDependency {
  std::string package;
  Version     version;

  /// @brief For each target, linking can be static or dynamic
  std::vector<std::pair<std::string, Link>> targets;

  [[nodiscard]] RegistryTargetDependency toRegistryTargetDependency() const
  {
    std::vector<std::string> str_targets;
    str_targets.reserve(targets.size());
    for (const auto &[k, v] : targets)
      if (v == Link::Dynamic)
        str_targets.push_back(k);

    return RegistryTargetDependency{
      .package = package,
      .version = version,
      .targets = std::move(str_targets),
    };
  }

  bool operator==(const ProjectTargetDependency &) const = default;
};

inline void to_json(json &j, const ProjectTargetDependency &d)
{
  j = json{
    { "package", d.package },
    { "version", d.version },
    { "targets", d.targets },
  };
}

inline void from_json(const json &j, ProjectTargetDependency &d)
{
  get_key(j, "package", d.package);
  get_key(j, "version", d.version);
  get_key(j, "targets", d.targets);
}

} // namespace zc::core
