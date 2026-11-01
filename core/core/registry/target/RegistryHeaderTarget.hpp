#pragma once

#include <string_view>

#include "RegistryTarget.hpp"

namespace zc::core
{

struct RegistryHeaderTarget final : RegistryTarget {
  [[nodiscard]] RegistryTargetType type() const override { return RegistryTargetType::Header; }

  [[nodiscard]] std::expected<void, Exception>
  addSymlinks(const std::string_view pkg_name, const stdfs::path &snapshot_dir) const override
  {
    return addHeaderSymlink(pkg_name, snapshot_dir);
  }

  [[nodiscard]] std::expected<void, Exception>
  removeSymlinks(const std::string_view pkg_name) const override
  {
    return removeHeaderSymlink(pkg_name);
  }

  [[nodiscard]] constexpr bool canBeADependency() const noexcept override { return true; }
};

} // namespace zc::core
