#pragma once

#include <vector>

#include "core/registry/target/RegistryTarget.hpp"
#include "framework/Version.hpp"

namespace zc::core
{

/// @brief Represents an installed version of a package
struct RegistryProject {
  /// @brief Project description
  std::string description;

  /// @brief Project author
  std::string author;

  /// @brief Project version
  Version version;

  /// @brief Project targets
  std::vector<std::unique_ptr<RegistryTarget>> targets;

  [[nodiscard]] std::expected<void, Exception> uninstallTargets() const
  {
    for (const auto &t : targets) {
      auto uninstall_res = t->uninstall();
      if (!uninstall_res)
        return std::unexpected(uninstall_res.error());
    }
    return {};
  }
};

inline void to_json(json &j, const RegistryProject &p)
{
  j = json{
    { "description", p.description },
    { "author", p.author },
    { "version", p.version },
    { "targets", p.targets },
  };
}

inline void from_json(const json &j, RegistryProject &p)
{
  get_key(j, "description", p.description);
  get_key(j, "author", p.author);
  get_key(j, "version", p.version);
  get_key(j, "targets", p.targets);
}

} // namespace zc::core
