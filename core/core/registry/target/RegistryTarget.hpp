#pragma once

#include <string>

#include "core/registry/target/RegistryTargetDependency.hpp"
#include "framework/util/types.hpp"
#include "RegistryTargetType.hpp"

namespace zc::core
{

/// @brief Contains the information of a target that is stored once this target is installed
struct RegistryTarget {
  /// @brief Target name
  std::string name;

  /// @brief Target include dirs (only public ones are kept)
  std::vector<std::string> inc_dirs;

  /// @brief Target macros (only public ones are kept)
  std::map<std::string, std::string> macros;

  /// @brief Internal dependencies (other targets from same package)
  std::vector<std::string> internal_deps;

  /// @brief External dependencies (targets from other packages)
  std::vector<RegistryTargetDependency> external_deps;

  RegistryTarget()                                  = default;
  RegistryTarget(RegistryTarget &&)                 = default;
  RegistryTarget(const RegistryTarget &)            = default;
  RegistryTarget &operator=(RegistryTarget &&)      = default;
  RegistryTarget &operator=(const RegistryTarget &) = default;
  virtual ~RegistryTarget()                         = default;

  [[nodiscard]] virtual std::string_view type() const              = 0;
  [[nodiscard]] virtual std::expected<void, Exception> uninstall() = 0;
};

inline void to_json(json &j, const std::unique_ptr<RegistryTarget> &t)
{
  if (!t) {
    j = nullptr;
    return;
  }
  j = json{
    { "type", t->type() },
    { "name", t->name },
    { "inc_dirs", t->inc_dirs },
    { "internal_deps", t->internal_deps },
    { "external_deps", t->external_deps },
    { "macros", t->macros },
  };
}

inline void from_json(const json &j, std::unique_ptr<RegistryTarget> &t)
{
  auto type = RegistryTargetType::Undef;
  get_key(j, "type", type);

  switch (type) {
    // TODO: add subclasses
  default:
    t = nullptr;
  }
  if (!t)
    return;

  get_key(j, "name", t->name);
  get_key(j, "inc_dirs", t->inc_dirs);
  get_key(j, "internal_deps", t->internal_deps);
  get_key(j, "external_deps", t->external_deps);
  get_key(j, "macros", t->macros);
}

} // namespace zc::core
