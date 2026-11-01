#pragma once

#include <expected>
#include <string>

#include "core/registry/target/RegistryTargetDependency.hpp"
#include "core/registry/target/RegistryTargetType.hpp"

namespace zc
{

class Exception;

namespace core
{

/// @brief Contains the information of a target that is stored once this target is installed
struct RegistryTarget {
  /// @brief Target name
  std::string name;

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

  virtual void writeToJson(json &j) const;

  virtual void loadFromJson(const json &j);

  [[nodiscard]] virtual RegistryTargetType type() const = 0;

  [[nodiscard]] virtual constexpr bool canBeADependency() const noexcept = 0;

  [[nodiscard]] virtual std::expected<void, Exception>
  addSymlinks(std::string_view pkg_name, const stdfs::path &snapshot_dir) const = 0;

  [[nodiscard]] virtual std::expected<void, Exception> removeSymlinks(std::string_view pkg_name) const = 0;

protected:
  [[nodiscard]] std::expected<void, Exception>
  addHeaderSymlink(std::string_view pkg_name, const stdfs::path &snapshot_dir) const;

  [[nodiscard]] std::expected<void, Exception> removeHeaderSymlink(std::string_view pkg_name) const;
};

void to_json(json &j, const std::unique_ptr<RegistryTarget> &t);

void from_json(const json &j, std::unique_ptr<RegistryTarget> &t);

} // namespace core
} // namespace zc
