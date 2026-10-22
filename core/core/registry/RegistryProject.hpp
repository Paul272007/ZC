#pragma once

#include <vector>

#include "core/registry/target/RegistryTarget.hpp"
#include "core/util/directories.hpp"
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

  [[nodiscard]] std::expected<void, Exception> createSymlinks(const std::string_view name) const
  {
    const stdfs::path snapshot_dir = cache_dir() / name / version.string();
    for (const auto &t : targets)
      if (auto res = t->addSymlinks(name, snapshot_dir); !res)
        return res;
    return {};
  }

  [[nodiscard]] std::expected<void, Exception> removeSymlinks(const std::string_view name) const
  {
    for (const auto &t : targets)
      if (auto res = t->removeSymlinks(name); !res)
        return res;
    return {};
  }
};

inline void to_json(json &j, const RegistryProject &p)
{
  j = json{
    { "description", p.description },
    { "author", p.author },
    { "version", p.version },
  };

  j["targets"] = json::object();
  for (const auto &t : p.targets)
    j["targets"][t->name] = t;
}

inline void from_json(const json &j, RegistryProject &p)
{
  get_key(j, "description", p.description);
  get_key(j, "author", p.author);
  get_key(j, "version", p.version);

  if (jsonContainsObject(j, "targets")) {
    for (const auto &[k, v] : j["targets"].items()) {
      auto target  = v.get<std::unique_ptr<RegistryTarget>>();
      target->name = k;
      p.targets.push_back(std::move(target));
    }
  }
}

} // namespace zc::core
