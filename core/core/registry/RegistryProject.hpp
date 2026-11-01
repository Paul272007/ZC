#pragma once

#include <string_view>
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

  [[nodiscard]] const RegistryTarget *findTarget(std::string_view name) const noexcept
  {
    for (const auto &t : targets)
      if (t->name == name)
        return t.get();
    return nullptr;
  }

  [[nodiscard]] bool hasTarget(std::string_view name) const noexcept { return findTarget(name) != nullptr; }
};

inline void to_json(json &j, const RegistryProject &p)
{
  j = json{
    { "description", p.description },
    { "author", p.author },
  };

  j["targets"] = json::object();
  for (const auto &t : p.targets)
    j["targets"][t->name] = t;
}

inline void from_json(const json &j, RegistryProject &p)
{
  get_optional_key(j, "author", p.author);
  get_optional_key(j, "description", p.description);

  if (jsonContainsObject(j, "targets")) {
    for (const auto &[k, v] : j["targets"].items()) {
      auto target  = v.get<std::unique_ptr<RegistryTarget>>();
      target->name = k;
      p.targets.push_back(std::move(target));
    }
  }
}

} // namespace zc::core
